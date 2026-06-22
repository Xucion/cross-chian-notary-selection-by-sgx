# cross-chain notary selection by SGX

这是一个基于 `vue-admin-template` 的前端后台示例，结合了 MySQL 身份认证服务、FISCO BCOS 智能合约和 Intel SGX 可信执行模块。

## 项目结构

- `src/` - Vue.js 前端管理后台代码
- `server/` - Express + MySQL 身份认证服务
- `contracts/` - FISCO BCOS Solidity 智能合约
- `SGX/` - Intel SGX 示例程序和 enclaves
- `tests/` - 单元测试

## 主要功能

- 基于 Vue 2 的管理后台界面
- 用户注册 / 登录 / JWT 认证
- 智能合约相关页面和区块链交互逻辑
- SGX 模拟模式下的可信执行演示

## 先决条件

- Node.js 8+ / npm
- MySQL
- Intel SGX SDK 或 SGX 模拟环境
- `sudo` 权限（SGX 程序运行时需要）

## 前端启动

```bash
# 安装依赖
npm install

# 启动开发模式
npm run dev
```

默认访问: `http://localhost:9528`

## 后端认证服务

`server/` 目录是一个最小的 Express + MySQL 身份认证服务，用于用户注册与登录。

1. 在 MySQL 中运行 `server/schema.sql` 创建数据表。
2. 复制 `server/.env.example` 为 `server/.env`，并填写数据库连接信息。
3. 安装依赖并启动服务：

```bash
cd server
npm install
npm run start
```

默认服务地址: `http://127.0.0.1:3001`

### 可用接口

- `POST /api/auth/register`：注册用户，参数 `{ username, password, userAddress }`
- `POST /api/auth/login`：登录，参数 `{ username, password }`，返回 `{ token }`
- `GET /api/auth/me`：获取用户信息，使用 `X-Token: <token>` 或 `Authorization: Bearer <token>`

## SGX 程序运行

SGX 执行说明位于 `SGX/howtouse.txt`，当前项目中可以使用 SGX 模拟模式。

```bash
cd SGX
sudo make clean
sudo make SGX_MODE=SIM
sudo ./app
```

或直接：

```bash
cd SGX
sudo make clean && sudo make SGX_MODE=SIM
```

## 智能合约

`contracts/` 目录包含 FISCO BCOS 的 Solidity 合约：

- `Count.sol`
- `FiscoOracleClient.sol`
- `OracleCore.sol`
- `OracleCoreInterface.sol`
- `Ownable.sol`
- `Register.sol`
- `SafeMath.sol`
- `TrustManagement.sol`

这些合约与前端数据交互逻辑配合使用，可用于链上用户地址映射、信任管理和预言机调用。

## 开发与测试

```bash
# 代码格式检查
npm run lint

# 单元测试
npm run test:unit

# 代码格式检查并自动修复
npm run lint -- --fix
```

## 备注

- 前端代码基于 `vue-admin-template`，默认使用 Element UI、axios、vue-router 和 vuex
- SGX 程序使用模拟模式运行，实际部署时请根据 Intel SGX SDK 进行配置

## License

本项目遵循 MIT 许可证。
