import request from '@/utils/fisco-request'

function reqTransHandle({
  groupId = 1,
  userAddress,
  contractName,
  contractAddress,
  funcName,
  funcParam = [],
  abi,
  useAes = false,
  // WeBASE-Front trans/handle optional fields (align with console requests)
  useCns = false,
  cnsName = '',
  version = '',
  contractPath = '/'
}) {
  return request({
    url: '/webase-api/trans/handle',
    method: 'post',
    data: {
      // Some WeBASE-Front versions treat groupId as string (console uses "1")
      groupId: String(groupId),
      user: userAddress,
      contractName,
      contractAddress,
      funcName,
      funcParam,
      contractAbi: abi,
      useAes,
      useCns,
      cnsName,
      version,
      contractPath
    }
  })
}

// HelloWorld
export function reqSendTransaction(data) {
  return reqTransHandle({
    userAddress: data.userAddress,
    contractName: 'HelloWorld',
    contractAddress: data.contractAddress,
    funcName: 'set',
    funcParam: [data.newString],
    abi: data.abi
  })
}

export function reqQueryContract(data) {
  return reqTransHandle({
    userAddress: data.userAddress,
    contractName: 'HelloWorld',
    contractAddress: data.contractAddress,
    funcName: 'get',
    funcParam: [],
    abi: data.abi
  })
}

// Register
export function reqRegisterStoreCipherData(data) {
  return reqTransHandle({
    userAddress: data.userAddress,
    contractName: 'Register',
    contractAddress: data.contractAddress,
    funcName: 'storeCipherData',
    funcParam: [data.ip, data.ciphertext, data.iv, data.tag, data.aad, data.pub],
    abi: data.abi
  })
}

export function reqRegisterResult(data) {
  return reqTransHandle({
    userAddress: data.userAddress,
    contractName: 'Register',
    contractAddress: data.contractAddress,
    funcName: 'result',
    funcParam: [],
    abi: data.abi
  })
}

export function reqRegisterGetTrustValuebyId(data) {
  return reqTransHandle({
    userAddress: data.userAddress,
    contractName: 'Register',
    contractAddress: data.contractAddress,
    funcName: 'getTrustValuebyId',
    funcParam: [data.id],
    abi: data.abi
  })
}

export function reqRegisterRegisterlist(data) {
  return reqTransHandle({
    userAddress: data.userAddress,
    contractName: 'Register',
    contractAddress: data.contractAddress,
    funcName: 'registerlist',
    funcParam: [data.id],
    abi: data.abi
  })
}

// Count
export function reqCountAdd(data) {
  return reqTransHandle({
    userAddress: data.userAddress,
    contractName: 'Count',
    contractAddress: data.contractAddress,
    funcName: 'add',
    funcParam: [data.assetIndex, data.receivingAssetIndex, data.receiverAddress, data.value],
    abi: data.abi
  })
}

export function reqCountGetTotal(data) {
  return reqTransHandle({
    userAddress: data.userAddress,
    contractName: 'Count',
    contractAddress: data.contractAddress,
    funcName: 'getTotal',
    funcParam: [],
    abi: data.abi
  })
}

export function reqCountRound(data) {
  return reqTransHandle({
    userAddress: data.userAddress,
    contractName: 'Count',
    contractAddress: data.contractAddress,
    funcName: 'round',
    funcParam: [],
    abi: data.abi
  })
}

export function reqCountReceiverLastRound(data) {
  return reqTransHandle({
    userAddress: data.userAddress,
    contractName: 'Count',
    contractAddress: data.contractAddress,
    funcName: 'receiverLastRound',
    funcParam: [data.receiverAddress],
    abi: data.abi
  })
}

export function reqCountTxInfoByRound(data) {
  return reqTransHandle({
    userAddress: data.userAddress,
    contractName: 'Count',
    contractAddress: data.contractAddress,
    funcName: 'Txid_to_Txinfo',
    funcParam: [data.round],
    abi: data.abi
  })
}

export function reqCountStartTime(data) {
  return reqTransHandle({
    userAddress: data.userAddress,
    contractName: 'Count',
    contractAddress: data.contractAddress,
    funcName: 'StartTime',
    funcParam: [],
    abi: data.abi
  })
}

// TrustManagement
export function reqTrustReadTrustValue(data) {
  return reqTransHandle({
    userAddress: data.userAddress,
    contractName: 'TrustManagement',
    contractAddress: data.contractAddress,
    funcName: 'readTrustValue',
    funcParam: [data.id],
    abi: data.abi
  })
}

export function reqTrustReadTxInfo(data) {
  return reqTransHandle({
    userAddress: data.userAddress,
    contractName: 'TrustManagement',
    contractAddress: data.contractAddress,
    funcName: 'readTxInfo',
    funcParam: [data.round],
    abi: data.abi
  })
}

export function reqTrustStartVote(data) {
  return reqTransHandle({
    userAddress: data.userAddress,
    contractName: 'TrustManagement',
    contractAddress: data.contractAddress,
    funcName: 'startVote',
    funcParam: [data.round, data.seq, data.proposal],
    abi: data.abi
  })
}

export function reqTrustVote(data) {
  return reqTransHandle({
    userAddress: data.userAddress,
    contractName: 'TrustManagement',
    contractAddress: data.contractAddress,
    funcName: 'vote',
    funcParam: [data.round, data.seq, data.notaryId, data.agree],
    abi: data.abi
  })
}

export function reqTrustBanned(data) {
  return reqTransHandle({
    userAddress: data.userAddress,
    contractName: 'TrustManagement',
    contractAddress: data.contractAddress,
    funcName: 'banned',
    funcParam: [data.address],
    abi: data.abi
  })
}
