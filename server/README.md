# Auth Server (MySQL)

This is a minimal Express + MySQL auth service for:
- Register: username/password ↔ chain `userAddress`
- Login: returns a JWT token
- Me: returns user info + mapped `userAddress`

## Setup

1. Create DB / table:
   - Run `server/schema.sql` in MySQL.
2. Configure env:
   - Copy `server/.env.example` to `server/.env` and fill values.
3. Install & run:
   - `cd server`
   - `npm install`
   - `npm run start`

Server default: `http://127.0.0.1:3001`

## Endpoints

- `POST /api/auth/register` `{ username, password, userAddress }`
- `POST /api/auth/login` `{ username, password }` -> `{ token }`
- `GET /api/auth/me` header `X-Token: <token>` or `Authorization: Bearer <token>`

