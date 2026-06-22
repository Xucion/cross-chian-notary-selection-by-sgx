import axios from 'axios'
import { Message } from 'element-ui'

// 创建专门用于 FISCO BCOS (WeBASE) 的 axios 实例
const fiscoRequest = axios.create({
  baseURL: '', // 关键：留空，不使用 /dev-api 前缀，让请求直接从 /webase-api 开始
  timeout: 10000 // 区块链共识打包可能需要一点时间，超时设为 10 秒
})

// 响应拦截器
fiscoRequest.interceptors.response.use(
  response => {
    // WeBASE-Front 返回的数据结构直接放行，不做 code: 20000 的死板校验
    return response.data
  },
  error => {
    console.error('WeBASE 请求出错:', error)
    const silent = error && error.config && error.config.silent
    if (!silent) {
      const status = error && error.response && error.response.status
      const respData = error && error.response && error.response.data
      let detail = ''
      if (typeof respData === 'string') detail = respData
      else if (respData && typeof respData === 'object') {
        detail = respData.message || respData.errorMessage || respData.error || ''
        if (!detail) {
          try {
            detail = JSON.stringify(respData)
          } catch (e) {
            detail = ''
          }
        }
      }
      const prefix = status ? `HTTP ${status}: ` : ''
      Message({
        message: `${prefix}${detail || error.message || '区块链请求报错'}`,
        type: 'error',
        duration: 5 * 1000
      })
    }
    return Promise.reject(error)
  }
)

export default fiscoRequest
