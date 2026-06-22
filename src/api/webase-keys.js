import request from '@/utils/fisco-request'

// WeBASE-Front Key API (proxied by /webase-api -> /WeBASE-Front)
// Docs: GET /privateKey/localKeyStores
export function reqListLocalKeyStores(options = {}) {
  return request({
    url: '/webase-api/privateKey/localKeyStores',
    method: 'get',
    ...options
  })
}

