import dotenv from "dotenv";
import { native, Pool } from "pg";
import fetch from "node-fetch";
import allUserIds from "./userIdss.json";
dotenv.config();

type User = {
  uuid: string;
  username: string;
  email: string;
  items: Record<
    string,
    {
      status: number;
      note?: string;
      t?: number;
    }
  >;
  metadata: {
    profileText?: string;
    enableCatalogCheckbox?: boolean;
  };
};

const PG_SCHEMA = "nook_exchange";

async function migrateUser(pool: Pool, user: User) {
  try {
    let userResult = await pool.query(
      `INSERT INTO ${PG_SCHEMA}.users (id, username, email, profile_text, settings) VALUES ($1, $2, $3, $4, $5)
       ON CONFLICT (id) DO UPDATE SET username=EXCLUDED.username, email=EXCLUDED.email, profile_text=EXCLUDED.profile_text, settings=EXCLUDED.settings`,
      [
        user.uuid,
        user.username,
        user.email !== "" ? user.email : null,
        user.metadata.profileText,
        JSON.stringify(
          user.metadata.enableCatalogCheckbox ? { enableCatalog: true } : {}
        ),
      ]
    );
    if (userResult.rowCount !== 1) {
      console.error("User upsert failed", user.uuid);
      throw new Error();
    }
    const deleteItemResult = await pool.query(
      `DELETE FROM ${PG_SCHEMA}.items WHERE user_id=$1`,
      [user.uuid]
    );
    const itemKeys = Object.keys(user.items);
    const itemResult = await pool.query(
      `INSERT INTO ${PG_SCHEMA}.items (user_id, item_id, variant, status, note, update_time)
       SELECT * FROM UNNEST ($1::uuid[], $2::text[], $3::int[], $4::int[], $5::text[], $6::timestamptz[])
       ON CONFLICT (user_id, item_id, variant) DO UPDATE SET status=EXCLUDED.status, note=EXCLUDED.note, update_time=EXCLUDED.update_time`,
      [
        itemKeys.map((_) => user.uuid),
        itemKeys.map((itemKey) => itemKey.split("@@")[0]),
        itemKeys.map((itemKey) => parseInt(itemKey.split("@@")[1])),
        itemKeys.map((itemKey) => user.items[itemKey].status),
        itemKeys.map((itemKey) => user.items[itemKey].note),
        itemKeys.map((itemKey) => {
          const t = user.items[itemKey].t;
          return t != null ? new Date(t * 1000) : null;
        }),
      ]
    );
    if (itemResult.rowCount !== itemKeys.length) {
      console.error(
        "mismatch item update count",
        user.uuid,
        itemResult.rowCount,
        itemKeys.length
      );
    }
  } catch (e) {
    console.error("ERROR for user", user.uuid, e);
  }
}

async function migrateUsers(userIds: string[]) {
  const response = await fetch(
    "https://paulshen-animalcrossing.builtwithdark.com/paul/userss?secret=natto&userIds=" +
      userIds.join(","),
    {
      method: "GET",
    }
  );
  const json = await response.json();
  const pool = new native!.Pool({
    max: 10,
    user: process.env.POSTGRES_USER,
    host: process.env.POSTGRES_HOST,
    password: process.env.POSTGRES_PASSWORD,
    database: process.env.POSTGRES_NAME,
    port: parseInt(process.env.POSTGRES_PORT!),
  });

  await Promise.all(json.map((user: User) => migrateUser(pool, user)));
  pool.end();
}

// const userIds = [
//   "2b8f02b3-b359-430b-ba8f-78ddcb2a3fd9",
//   "c8e0fe73-97c4-4a1b-9d55-d61748e0ced7",
//   "3721248d-3439-4101-ae60-4ca52b574fc9",
// ];
// migrateUsers(userIds);

const BATCH_SIZE = 25;
const totalNumBatches = allUserIds.length / BATCH_SIZE + 1;
console.log("total batches", totalNumBatches);
async function processBatch(offset: number) {
  if (offset > totalNumBatches) {
    return;
  }
  console.log("processing batch", offset);
  await Promise.all([
    migrateUsers(
      allUserIds.slice(offset * BATCH_SIZE, (offset + 1) * BATCH_SIZE)
    ),
    migrateUsers(
      allUserIds.slice((offset + 1) * BATCH_SIZE, (offset + 2) * BATCH_SIZE)
    ),
  ]);
  processBatch(offset + 2);
}

processBatch(0);
