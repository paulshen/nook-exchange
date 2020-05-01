import express from "express";
import bearerToken from "express-bearer-token";
import { Pool, Client } from "pg";
import dotenv from "dotenv";
import bodyParser from "body-parser";
import crypto from "crypto";

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

app.get("/paullikesnatto", (req, res) => {
  res.send("hrm");
});

app.post("/register", async (req, res) => {
  const { userId, username, password, email, createTime, sessionId } = req.body;
  const passwordSalt = generatePasswordSalt();
  const passwordHash = hashPassword(password, passwordSalt);
  const client = await pool.connect();
  const userResult = await client.query(
    `INSERT INTO ${PG_SCHEMA}.users (id, username, email, password, password_salt, create_time) VALUES ($1, $2, $3, $4, $5, $6)`,
    [userId, username, email, passwordHash, passwordSalt, createTime]
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
});

app.post("/sessions", async (req, res) => {
  const { sessionId, userId, createTime } = req.body;
  const client = await pool.connect();
  const insertResult = await client.query(
    `INSERT INTO ${PG_SCHEMA}.sessions (id, user_id, create_time) VALUES ($1, $2, $3)`,
    [sessionId, userId, createTime]
  );
  if (insertResult.rowCount === 1) {
    res.sendStatus(201);
  } else {
    res.sendStatus(400);
  }
});

app.post("/@me4/items/:itemId/:variant/status", async (req, res) => {
  const sessionId = req.token;
  const client = await pool.connect();
  let errorStatusCode;
  try {
    const sessionResult = await client.query(
      `SELECT * FROM ${PG_SCHEMA}.sessions WHERE id=$1`,
      [sessionId]
    );
    if (sessionResult.rows.length === 1) {
      const userId = sessionResult.rows[0]["user_id"];
      // TODO assertions
      const updateResult = await client.query(
        `INSERT INTO ${PG_SCHEMA}.items (user_id, item_id, variant, status, update_time) VALUES ($1, $2, $3, $4, NOW())
         ON CONFLICT (user_id, item_id, variant) DO UPDATE SET status=EXCLUDED.status, update_time=EXCLUDED.update_time`,
        [userId, req.params.itemId, req.params.variant, req.body.status]
      );
      if (updateResult.rowCount !== 1) {
        console.error("Unexpected rowCount", updateResult.rowCount);
      }
    } else {
      errorStatusCode = 401;
    }
  } catch (e) {
    console.error(e);
  } finally {
    client.release();
  }
  res.sendStatus(errorStatusCode !== undefined ? errorStatusCode : 201);
});

app.post("/@me4/items/:itemId/batch/status", async (req, res) => {
  const sessionId = req.token;
  const client = await pool.connect();
  let errorStatusCode;
  try {
    const sessionResult = await client.query(
      `SELECT * FROM ${PG_SCHEMA}.sessions WHERE id=$1`,
      [sessionId]
    );
    if (sessionResult.rows.length === 1) {
      const userId = sessionResult.rows[0]["user_id"];
      const status: number = req.body.status;
      const variants: Array<number> = req.body.variants;
      // TODO assertions
      await Promise.all(
        variants.map((variant) => {
          client.query(
            `INSERT INTO ${PG_SCHEMA}.items (user_id, item_id, variant, status, update_time) VALUES ($1, $2, $3, $4, NOW())
         ON CONFLICT (user_id, item_id, variant) DO UPDATE SET status=EXCLUDED.status, update_time=EXCLUDED.update_time`,
            [userId, req.params.itemId, variant, status]
          );
        })
      );
    } else {
      errorStatusCode = 401;
    }
  } catch (e) {
    console.error(e);
    errorStatusCode = 400;
  } finally {
    client.release();
  }
  res.sendStatus(errorStatusCode !== undefined ? errorStatusCode : 201);
});

app.post("/@me4/items/:itemId/:variant/note", async (req, res) => {
  const sessionId = req.token;
  const client = await pool.connect();
  let errorStatusCode;
  try {
    const sessionResult = await client.query(
      `SELECT * FROM ${PG_SCHEMA}.sessions WHERE id=$1`,
      [sessionId]
    );
    if (sessionResult.rows.length === 1) {
      const userId = sessionResult.rows[0]["user_id"];
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
    } else {
      errorStatusCode = 401;
    }
  } catch (e) {
    console.error(e);
  } finally {
    client.release();
  }
  res.sendStatus(errorStatusCode !== undefined ? errorStatusCode : 201);
});

app.delete("/@me3/items/:itemId/:variant", async (req, res) => {
  const sessionId = req.token;
  const client = await pool.connect();
  let errorStatusCode;
  try {
    const sessionResult = await client.query(
      `SELECT * FROM ${PG_SCHEMA}.sessions WHERE id=$1`,
      [sessionId]
    );
    if (sessionResult.rows.length === 1) {
      const userId = sessionResult.rows[0]["user_id"];
      // TODO assertions
      const deleteResult = await client.query(
        `DELETE FROM ${PG_SCHEMA}.items WHERE user_id=$1 AND item_id=$2 AND variant=$3`,
        [userId, req.params.itemId, req.params.variant]
      );
    } else {
      errorStatusCode = 401;
    }
  } catch (e) {
    console.error(e);
  } finally {
    client.release();
  }
  res.sendStatus(errorStatusCode !== undefined ? errorStatusCode : 204);
});

app.listen(3022);
console.log("Listening at 3022");
