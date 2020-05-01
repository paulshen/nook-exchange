import inquirer from "inquirer";
import { Client } from "pg";
import dotenv from "dotenv";
import crypto from "crypto";

dotenv.config();

const PG_SCHEMA = "nook_exchange";

function hashPassword(password: string, salt: string) {
  var hash = crypto.createHmac("sha512", salt);
  hash.update(password);
  return hash.digest("hex");
}

async function main() {
  let client: Client | undefined;
  let userRow: any;
  inquirer
    .prompt([
      {
        type: "input",
        name: "username",
      },
    ])
    .then(async ({ username }) => {
      client = new Client({
        user: process.env.POSTGRES_USER,
        host: process.env.POSTGRES_HOST,
        password: process.env.POSTGRES_PASSWORD,
        database: process.env.POSTGRES_NAME,
        port: parseInt(process.env.POSTGRES_PORT!),
      });
      await client.connect();
      const userResult = await client.query(
        `SELECT * FROM ${PG_SCHEMA}.users WHERE lower(username)=$1`,
        [username.toLowerCase()]
      );
      userRow = userResult.rows[0];
      if (userRow == null) {
        console.error(`Could not find user with username: ${username}`);
        return Promise.reject();
      }
      return inquirer.prompt([
        {
          type: "confirm",
          name: "emailConfirm",
          message: `Is this the right email? (${userRow.email})`,
        },
      ]);
    })
    .then(({ emailConfirm }) => {
      if (!emailConfirm) {
        return Promise.reject();
      }

      return inquirer.prompt([
        {
          type: "input",
          name: "password",
          message: "What is the new password?",
        },
      ]);
    })
    .then(async ({ password }) => {
      const passwordHash = hashPassword(password, userRow!["password_salt"]);
      await client!.query(
        `UPDATE ${PG_SCHEMA}.users SET password=$1 WHERE id=$2`,
        [passwordHash, userRow!["id"]]
      );
      console.log("Success!");
    })
    .catch((e) => {
      console.error(e);
    })
    .finally(() => {
      if (client != null) {
        client.end();
      }
    });
}

main();
