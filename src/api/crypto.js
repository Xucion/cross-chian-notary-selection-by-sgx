import request from '@/utils/request'

export function reqEncodeCipherData(data) {
  return request({
    url: '/crypto/encode',
    method: 'post',
    data
  })
}

