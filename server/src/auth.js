const jwt = require('jsonwebtoken')

function requiredEnv(name, fallback) {
  const v = process.env[name] != null ? String(process.env[name]) : ''
  if (v) return v
  if (fallback != null) return fallback
  throw new Error(`Missing env: ${name}`)
}

const JWT_SECRET = requiredEnv('JWT_SECRET')
const JWT_EXPIRES_IN = requiredEnv('JWT_EXPIRES_IN', '7d')

function signToken(payload) {
  return jwt.sign(payload, JWT_SECRET, { expiresIn: JWT_EXPIRES_IN })
}

function getTokenFromRequest(req) {
  const auth = req.headers.authorization
  if (auth && typeof auth === 'string' && auth.toLowerCase().startsWith('bearer ')) {
    return auth.slice(7).trim()
  }
  const xToken = req.headers['x-token']
  if (xToken && typeof xToken === 'string') return xToken.trim()
  return null
}

function requireAuth(req, res, next) {
  const token = getTokenFromRequest(req)
  if (!token) {
    return res.status(401).json({ code: 50008, message: 'Missing token' })
  }
  try {
    const decoded = jwt.verify(token, JWT_SECRET)
    req.user = decoded
    return next()
  } catch (e) {
    return res.status(401).json({ code: 50014, message: 'Token expired or invalid' })
  }
}

module.exports = {
  signToken,
  requireAuth
}

