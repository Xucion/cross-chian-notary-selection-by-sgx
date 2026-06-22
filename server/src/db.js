const mysql = require('mysql2/promise')

function requiredEnv(name, fallback) {
  const v = process.env[name] != null ? String(process.env[name]) : ''
  if (v) return v
  if (fallback != null) return fallback
  throw new Error(`Missing env: ${name}`)
}

const pool = mysql.createPool({
  host: requiredEnv('DB_HOST'),
  port: Number(requiredEnv('DB_PORT', '3306')),
  user: requiredEnv('DB_USER'),
  password: requiredEnv('DB_PASSWORD'),
  database: requiredEnv('DB_DATABASE'),
  waitForConnections: true,
  connectionLimit: 10,
  queueLimit: 0
})

module.exports = { pool }

