const express = require('express')
const cors = require('cors')
const dotenv = require('dotenv')

dotenv.config()

const { pool } = require('./db')
const { signToken, requireAuth } = require('./auth')
const bcrypt = require('bcryptjs')
const path = require('path')
const { spawn } = require('child_process')

const app = express()
app.use(cors())
app.use(express.json({ limit: '1mb' }))

function ok(data) {
  return { code: 20000, data }
}

function bad(message, code) {
  return { code: code || 60204, message }
}

function normalizeUsername(username) {
  return String(username || '').trim()
}

function normalizeAddress(addr) {
  return String(addr || '').trim()
}

app.get('/api/health', (req, res) => {
  res.json(ok({ status: 'ok' }))
})

app.post('/api/auth/register', async(req, res) => {
  const username = normalizeUsername(req.body && req.body.username)
  const password = String((req.body && req.body.password) || '')
  const userAddress = normalizeAddress(req.body && req.body.userAddress)

  if (!username) return res.json(bad('Username is required.'))
  if (username.length < 3) return res.json(bad('Username must be at least 3 characters.'))
  if (!password || password.length < 6) return res.json(bad('Password must be at least 6 characters.'))
  if (!userAddress) return res.json(bad('UserAddress is required.'))

  const passwordHash = await bcrypt.hash(password, 10)

  try {
    await pool.execute(
      'INSERT INTO users (username, password_hash, user_address) VALUES (?, ?, ?)',
      [username, passwordHash, userAddress]
    )
    return res.json(ok({ success: true }))
  } catch (e) {
    if (e && e.code === 'ER_DUP_ENTRY') {
      return res.json(bad('Username already exists.'))
    }
    // eslint-disable-next-line no-console
    console.error(e)
    return res.status(500).json({ code: 50000, message: 'Server error' })
  }
})

app.post('/api/auth/login', async(req, res) => {
  const username = normalizeUsername(req.body && req.body.username)
  const password = String((req.body && req.body.password) || '')
  if (!username || !password) return res.json(bad('Account and password are incorrect.'))

  try {
    const [rows] = await pool.execute('SELECT id, username, password_hash FROM users WHERE username = ?', [username])
    const user = Array.isArray(rows) && rows.length ? rows[0] : null
    if (!user) return res.json(bad('Account and password are incorrect.'))

    const okPwd = await bcrypt.compare(password, user.password_hash)
    if (!okPwd) return res.json(bad('Account and password are incorrect.'))

    const token = signToken({ uid: user.id, username: user.username })
    return res.json(ok({ token }))
  } catch (e) {
    // eslint-disable-next-line no-console
    console.error(e)
    return res.status(500).json({ code: 50000, message: 'Server error' })
  }
})

app.get('/api/auth/me', requireAuth, async(req, res) => {
  try {
    const uid = req.user && req.user.uid
    const [rows] = await pool.execute('SELECT username, user_address AS userAddress, created_at AS createdAt FROM users WHERE id = ?', [uid])
    const user = Array.isArray(rows) && rows.length ? rows[0] : null
    if (!user) return res.status(401).json({ code: 50008, message: 'Login failed, unable to get user details.' })
    return res.json(ok({
      roles: ['admin'],
      name: user.username,
      avatar: 'https://wpimg.wallstcn.com/f778738c-e4f8-4870-b634-56703b4acafe.gif',
      userAddress: user.userAddress,
      createdAt: user.createdAt
    }))
  } catch (e) {
    // eslint-disable-next-line no-console
    console.error(e)
    return res.status(500).json({ code: 50000, message: 'Server error' })
  }
})

app.post('/api/auth/logout', (req, res) => {
  // Stateless JWT: client just discards token
  res.json(ok('success'))
})

app.post('/api/crypto/encode', requireAuth, async(req, res) => {
  const privateKeyHex = String((req.body && (req.body.private_key_hex || req.body.privateKeyHex)) || '').trim()
  const asset1 = String((req.body && (req.body.Asset1 || req.body.asset1)) || '').trim()
  const asset2 = String((req.body && (req.body.Asset2 || req.body.asset2)) || '').trim()
  const asset3 = String((req.body && (req.body.Asset3 || req.body.asset3)) || '').trim()

  if (!asset1 || !asset2 || !asset3) return res.json(bad('Asset1/Asset2/Asset3 are required.'))

  const scriptPath = path.resolve(__dirname, '../../Encode.py')
  const args = ['--asset1', asset1, '--asset2', asset2, '--asset3', asset3]
  if (privateKeyHex) args.push('--private_key_hex', privateKeyHex)

  const child = spawn('python3', [scriptPath, ...args], {
    stdio: ['ignore', 'pipe', 'pipe'],
    env: process.env
  })

  let stdout = ''
  let stderr = ''
  child.stdout.on('data', chunk => { stdout += chunk.toString() })
  child.stderr.on('data', chunk => { stderr += chunk.toString() })

  const timeoutMs = 15000
  const timer = setTimeout(() => {
    try { child.kill('SIGKILL') } catch (e) {}
  }, timeoutMs)

  child.on('close', code => {
    clearTimeout(timer)
    if (code !== 0) {
      return res.status(500).json({ code: 50000, message: (stderr || 'Encode failed').slice(0, 500) })
    }
    try {
      const data = JSON.parse(stdout)
      return res.json(ok(data))
    } catch (e) {
      return res.status(500).json({ code: 50000, message: 'Encode output is not valid JSON' })
    }
  })
})

const port = Number(process.env.PORT || 3001)
app.listen(port, () => {
  // eslint-disable-next-line no-console
  console.log(`[auth-server] listening on http://127.0.0.1:${port}`)
})
