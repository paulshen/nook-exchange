import express from "express";
import bearerToken from "express-bearer-token";
import { Pool, PoolClient } from "pg";
import dotenv from "dotenv";
import bodyParser from "body-parser";
import crypto from "crypto";
import cors from "cors";
import slugify from "slugify";

dotenv.config();

const pool = new Pool({
  user: process.env.POSTGRES_USER,
  host: process.env.POSTGRES_HOST,
  password: process.env.POSTGRES_PASSWORD,
  database: process.env.POSTGRES_NAME,
  port: parseInt(process.env.POSTGRES_PORT!),
});
const PG_SCHEMA = "nook_exchange";

function generatePasswordSalt() {
  const length = 16;
  return crypto
    .randomBytes(Math.ceil(length / 2))
    .toString("hex") /** convert to hexadecimal format */
    .slice(0, length); /** return required number of characters */
}

function hashPassword(password: string, salt: string) {
  var hash = crypto.createHmac("sha512", salt);
  hash.update(password);
  return hash.digest("hex");
}

const app = express();
app.use(bearerToken());
app.use(bodyParser.json());
var corsWhitelist = [
  "http://localhost:8080",
  "https://nook.exchange",
  "https://preview.nook.exchange",
];
var corsOptions = {
  origin: function (
    origin: string | undefined,
    callback: (error: any, success?: boolean) => void
  ) {
    if (corsWhitelist.indexOf(origin!) !== -1) {
      callback(null, true);
    } else {
      callback(new Error("Not allowed by CORS: " + origin!));
    }
  },
  credentials: true,
};
app.options("*", cors(corsOptions));

app.get("/paullikesnatto", (req, res) => {
  res.send("hrm");
});

app.post("/register", async (req, res) => {
  const { userId, username, password, email, createTime, sessionId } = req.body;
  const passwordSalt = generatePasswordSalt();
  const passwordHash = hashPassword(password, passwordSalt);
  const client = await pool.connect();
  try {
    const userResult = await client.query(
      `INSERT INTO ${PG_SCHEMA}.users (id, username, email, password, password_salt, create_time) VALUES ($1, $2, $3, $4, $5, $6)`,
      [
        userId,
        username,
        email !== "" ? email : null,
        passwordHash,
        passwordSalt,
        createTime,
      ]
    );
    if (userResult.rowCount === 1) {
      const sessionResult = await client.query(
        `INSERT INTO ${PG_SCHEMA}.sessions (id, user_id, create_time) VALUES ($1, $2, $3)`,
        [sessionId, userId, createTime]
      );
      res.sendStatus(201);
    } else {
      res.sendStatus(400);
    }
  } finally {
    client.release();
  }
});

app.post("/sessions", async (req, res) => {
  const { sessionId, userId, createTime } = req.body;
  const client = await pool.connect();
  try {
    const insertResult = await client.query(
      `INSERT INTO ${PG_SCHEMA}.sessions (id, user_id, create_time) VALUES ($1, $2, $3)`,
      [sessionId, userId, createTime]
    );
    if (insertResult.rowCount === 1) {
      res.sendStatus(201);
    } else {
      res.sendStatus(400);
    }
  } finally {
    client.release();
  }
});

async function validateUserId(
  userId: string,
  req: express.Request,
  client: PoolClient
) {
  const sessionId = req.token;
  if (!sessionId) {
    console.error("missing sessionId in req.token");
    return false;
  }
  const sessionResult = await client.query(
    `SELECT * FROM ${PG_SCHEMA}.sessions WHERE id=$1`,
    [sessionId]
  );
  if (sessionResult.rows.length === 0) {
    console.error("could not find sessionId", sessionId);
    return false;
  }
  if (
    sessionResult.rows.length === 1 &&
    sessionResult.rows[0]["user_id"] !== userId
  ) {
    console.error(
      "mismatch userId",
      sessionResult.rows[0]["user_id"] !== userId
    );
    return false;
  }
  return true;
}

app.post(
  "/@me4/items/:itemId/batch/status",
  cors(corsOptions),
  async (req, res) => {
    const client = await pool.connect();
    let errorStatusCode;
    try {
      const userId = req.body.userId;
      await validateUserId(userId, req, client);
      const itemId = req.params.itemId;
      const status: number = req.body.status;
      const variants: Array<number> = req.body.variants;
      // TODO assertions
      await Promise.all(
        variants.map((variant) => {
          client.query(
            `INSERT INTO ${PG_SCHEMA}.items (user_id, item_id, variant, status, update_time) VALUES ($1, $2, $3, $4, NOW())
            ON CONFLICT (user_id, item_id, variant) DO UPDATE SET status=EXCLUDED.status, update_time=EXCLUDED.update_time`,
            [userId, itemId, variant, status]
          );
        })
      );
    } catch (e) {
      console.error(e);
      errorStatusCode = 400;
    } finally {
      client.release();
    }
    res.sendStatus(errorStatusCode !== undefined ? errorStatusCode : 201);
  }
);

app.post(
  "/@me4/items/:itemId/:variant/status",
  cors(corsOptions),
  async (req, res) => {
    const sessionId = req.token;
    const client = await pool.connect();
    let errorStatusCode;
    try {
      const userId = req.body.userId;
      await validateUserId(userId, req, client);
      // TODO assertions
      const updateResult = await client.query(
        `INSERT INTO ${PG_SCHEMA}.items (user_id, item_id, variant, status, update_time) VALUES ($1, $2, $3, $4, NOW())
         ON CONFLICT (user_id, item_id, variant) DO UPDATE SET status=EXCLUDED.status, update_time=EXCLUDED.update_time`,
        [userId, req.params.itemId, req.params.variant, req.body.status]
      );
      if (updateResult.rowCount !== 1) {
        console.error("Unexpected rowCount", updateResult.rowCount);
      }
    } catch (e) {
      console.error(e);
    } finally {
      client.release();
    }
    res.sendStatus(errorStatusCode !== undefined ? errorStatusCode : 201);
  }
);

app.post(
  "/@me4/items/:itemId/:variant/note",
  cors(corsOptions),
  async (req, res) => {
    const client = await pool.connect();
    let errorStatusCode;
    try {
      const userId = req.body.userId;
      await validateUserId(userId, req, client);
      // TODO assertions
      const updateResult = await client.query(
        `INSERT INTO ${PG_SCHEMA}.items (user_id, item_id, variant, note, update_time) VALUES ($1, $2, $3, $4, NOW())
         ON CONFLICT (user_id, item_id, variant) DO UPDATE SET note=EXCLUDED.note, update_time=EXCLUDED.update_time`,
        [userId, req.params.itemId, req.params.variant, req.body.note]
      );
      if (updateResult.rowCount !== 1) {
        console.error("Unexpected rowCount", updateResult.rowCount);
        errorStatusCode = 400;
      }
    } catch (e) {
      console.error(e);
    } finally {
      client.release();
    }
    res.sendStatus(errorStatusCode !== undefined ? errorStatusCode : 201);
  }
);

app.delete(
  "/@me3/items/:itemId/:variant",
  cors(corsOptions),
  async (req, res) => {
    const client = await pool.connect();
    let errorStatusCode;
    try {
      const userId = req.body.userId;
      await validateUserId(userId, req, client);
      // TODO assertions
      const deleteResult = await client.query(
        `DELETE FROM ${PG_SCHEMA}.items WHERE user_id=$1 AND item_id=$2 AND variant=$3`,
        [userId, req.params.itemId, req.params.variant]
      );
    } catch (e) {
      console.error(e);
    } finally {
      client.release();
    }
    res.sendStatus(errorStatusCode !== undefined ? errorStatusCode : 204);
  }
);

function sanitizeUsername(username: string) {
  return slugify(username).replace('"', "").replace("'", "");
}

app.patch("/@me", cors(corsOptions), async (req, res) => {
  const { username, password, email, oldPassword } = req.body;
  const updates: Record<string, any> = {};
  if (oldPassword == null) {
    res.sendStatus(401);
    return;
  }
  if (username != null) {
    if (username.length < 3) {
      res.status(400).send("Username must be at least 3 characters");
      return;
    }
    if (sanitizeUsername(username) !== username) {
      res.status(400).send("Try username " + sanitizeUsername(username));
      return;
    }
  }
  if (password != null) {
    if (password.length < 6) {
      res.status(400).send("Password must be at least 6 characters");
      return;
    }
  }
  const client = await pool.connect();
  let errorStatusCode;
  let body;
  try {
    const userId = req.body.userId;
    const userResult = await client.query(
      `SELECT * FROM ${PG_SCHEMA}.users WHERE id=$1`,
      [userId]
    );
    if (userResult.rowCount === 0) {
      errorStatusCode = 401;
      throw new Error("oh no");
    }
    if (
      hashPassword(oldPassword, userResult.rows[0].password_salt) !==
      userResult.rows[0].password
    ) {
      errorStatusCode = 401;
      throw new Error("password");
    }
    if (username != null) {
      const usernameResult = await client.query(
        `SELECT * FROM ${PG_SCHEMA}.users WHERE lower(username)=$1`,
        [username.toLowerCase()]
      );
      if (usernameResult.rowCount > 0) {
        body = "Username already registered";
        errorStatusCode = 400;
        throw new Error();
      }
      updates["username"] = username;
    }
    if (password != null) {
      updates["password"] = hashPassword(
        password,
        userResult.rows[0].password_salt
      );
    }
    if (email != null) {
      const emailResult = await client.query(
        `SELECT * FROM ${PG_SCHEMA}.users WHERE email=$1`,
        [email]
      );
      if (emailResult.rowCount > 0) {
        body = "Email already registered";
        errorStatusCode = 400;
        throw new Error();
      }
      updates["email"] = email !== "" ? email : null;
    }
    const setPhrase = Object.keys(updates)
      .map((updateKey, i) => {
        return `${updateKey}=$${i + 2}`;
      })
      .join(", ");
    const updateResult = await client.query(
      `UPDATE ${PG_SCHEMA}.users SET ${setPhrase} WHERE id=$1`,
      [userId, ...Object.keys(updates).map((k) => updates[k])]
    );
    if (updateResult.rowCount !== 1) {
      errorStatusCode = 400;
    }
  } catch (e) {
    console.error(e);
  } finally {
    client.release();
  }
  res.status(errorStatusCode !== undefined ? errorStatusCode : 204).send(body);
});

app.listen(3022);
console.log("Listening at 3022");
