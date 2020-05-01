import express from "express";

const app = express();

app.get("/hello", (req, res) => {
  res.json({
    world: true,
  });
});

app.listen(3022);
