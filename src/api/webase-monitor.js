import request from '@/utils/fisco-request'

// WeBASE-Front OpenAPI (proxied by vue.config.js: /webase-api -> /WeBASE-Front)
// Examples:
// - GET /webase-api/{groupId}/web3/blockNumber
// - GET /webase-api/{groupId}/web3/transaction-total
// - GET /webase-api/{groupId}/web3/blockByNumber/{blockNumber}

export function reqGetBlockNumber(groupId = 1) {
  return request({
    url: `/webase-api/${groupId}/web3/blockNumber`,
    method: 'get'
  })
}

export function reqGetTransactionTotal(groupId = 1) {
  return request({
    url: `/webase-api/${groupId}/web3/transaction-total`,
    method: 'get'
  })
}

export function reqGetBlockByNumber(groupId = 1, blockNumber) {
  return request({
    url: `/webase-api/${groupId}/web3/blockByNumber/${blockNumber}`,
    method: 'get'
  })
}

// 获取交易信息（不同 WeBASE-Front 版本接口名不同，主流为 /web3/transaction/{hash}）
export function reqGetTransactionByHash(groupId = 1, txHash, options = {}) {
  return request({
    url: `/webase-api/${groupId}/web3/transaction/${txHash}`,
    method: 'get',
    ...options
  })
}

export function reqGetTransactionReceipt(groupId = 1, txHash, options = {}) {
  return request({
    url: `/webase-api/${groupId}/web3/transactionReceipt/${txHash}`,
    method: 'get',
    ...options
  })
}

// 兼容不同 WeBASE-Front 版本的 tx 查询接口命名
export async function reqGetTransactionByHashCompat(groupId = 1, txHash, options = {}) {
  const candidates = [
    `/webase-api/${groupId}/web3/transaction/${txHash}`,
    `/webase-api/${groupId}/web3/transactionByHash/${txHash}`,
    `/webase-api/${groupId}/web3/getTransactionByHash/${txHash}`
  ]

  let lastErr = null
  for (const url of candidates) {
    try {
      return await request({ url, method: 'get', ...options })
    } catch (e) {
      const status = e && e.response && e.response.status
      lastErr = e
      if (status === 404) continue
      throw e
    }
  }
  throw lastErr
}
