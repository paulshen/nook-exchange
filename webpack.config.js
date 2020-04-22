const HtmlWebpackPlugin = require("html-webpack-plugin");

module.exports = {
  mode: "development",
  entry: {
    Data: "./src/Data.js",
    Index: "./src/Index.bs.js",
  },
  devtool: "inline-source-map",
  devServer: {
    contentBase: "./s3",
    historyApiFallback: true,
  },
  output: {
    publicPath: "/",
  },
  module: {
    rules: [
      {
        test: /\.(png|jpe?g|gif)$/i,
        use: [
          {
            loader: "file-loader",
            options: {
              name: "[name].[contenthash:8].[ext]",
              publicPath: "/",
            },
          },
        ],
      },
    ],
  },
  plugins: [
    new HtmlWebpackPlugin({
      template: "./src/index.html",
    }),
  ],
};
