<template>
  <div class="app-container">
    <div class="feature-hero">
      <div class="feature-hero__left">
        <div class="feature-hero__title">TrustManagement</div>
        <div class="feature-hero__subtitle">投票治理与可信公证人选择</div>
      </div>
      <img class="feature-hero__img" :src="heroImg" alt="trust-management">
    </div>

    <el-card class="box-card" shadow="hover">
      <div slot="header" class="clearfix">
        <span>TrustManagement 合约 - 基础参数</span>
      </div>

      <el-form :inline="true" label-width="120px" @submit.native.prevent>
        <el-form-item label="User 地址">
          <el-input v-model="userAddress" placeholder="WeBASE 用户地址" clearable style="width: 420px;" />
        </el-form-item>
        <el-form-item label="合约地址">
          <el-input v-model="contractAddress" placeholder="TrustManagement 合约地址" clearable style="width: 420px;" />
        </el-form-item>
      </el-form>
    </el-card>

    <el-card class="box-card" shadow="hover" style="margin-top: 16px;">
      <div slot="header" class="clearfix">
        <span>通过公证人ID读取信任值</span>
      </div>

      <el-form :inline="true" label-width="120px" @submit.native.prevent>
        <el-form-item label="Trust id">
          <el-input-number v-model="trustId" :min="0" :step="1" controls-position="right" />
        </el-form-item>
        <el-form-item>
          <el-button type="success" :loading="isReadingTrust" @click="handleReadTrustValue">
            readTrustValue()
          </el-button>
        </el-form-item>
        <el-form-item label="Trust value">
          <el-tag type="info" size="medium">{{ trustValueDisplay }}</el-tag>
        </el-form-item>
      </el-form>
    </el-card>

    <el-card class="box-card" shadow="hover" style="margin-top: 16px;">
      <div slot="header" class="clearfix">
        <span>通过轮数读取交易信息</span>
      </div>

      <el-form :inline="true" label-width="120px" @submit.native.prevent>
        <el-form-item label="round">
          <el-input-number v-model="txRound" :min="0" :step="1" controls-position="right" />
        </el-form-item>
        <el-form-item>
          <el-button type="success" :loading="isReadingTxInfo" @click="handleReadTxInfo">
            readTxInfo()
          </el-button>
        </el-form-item>
      </el-form>

      <div v-if="txInfo" style="margin-top: 10px;">
        <div class="kv-table">
          <div class="kv-row">
            <div class="kv-key">Notary_id</div>
            <div class="kv-val"><span class="mono selectable">{{ txInfo.Notary_id || '-' }}</span></div>
          </div>
          <div class="kv-row">
            <div class="kv-key">Receivingchain_id</div>
            <div class="kv-val"><span class="mono selectable">{{ txInfo.Receivingchain_id }}</span></div>
          </div>
          <div class="kv-row">
            <div class="kv-key">Receiver_address</div>
            <div class="kv-val"><span class="mono selectable">{{ txInfo.Receiver_address || '-' }}</span></div>
          </div>
          <div class="kv-row">
            <div class="kv-key">total</div>
            <div class="kv-val"><span class="mono selectable">{{ txInfo.total || '-' }}</span></div>
          </div>
          <div class="kv-row">
            <div class="kv-key">T_select</div>
            <div class="kv-val"><span class="mono selectable">{{ txInfo.T_select }}</span></div>
          </div>
        </div>
      </div>
    </el-card>

    <el-card class="box-card" shadow="hover" style="margin-top: 16px;">
      <div slot="header" class="clearfix">
        <span>信誉管理投票</span>
      </div>

      <el-card shadow="never" style="margin-bottom: 12px; border: 1px solid #ebeef5;">
        <div slot="header" class="clearfix">
          <span>投票列表（前端记录）</span>
          <el-button style="float: right; padding: 3px 0; margin-left: 12px;" type="text" @click="clearLocalVotes">清空</el-button>
          <el-button style="float: right; padding: 3px 0" type="text" @click="loadLocalVotes">刷新</el-button>
        </div>

        <el-table :data="localVotes" size="mini" border style="width: 100%; margin-top: 10px;">
          <el-table-column prop="createdAt" label="time" width="160" />
          <el-table-column prop="creator" label="user" min-width="160" show-overflow-tooltip />
          <el-table-column prop="round" label="round" width="70" />
          <el-table-column prop="seq" label="seq" width="60" />
          <el-table-column prop="proposal" label="proposal" min-width="220" show-overflow-tooltip />
          <el-table-column prop="txHash" label="txHash" min-width="180" show-overflow-tooltip />
          <el-table-column label="操作" width="120" fixed="right">
            <template slot-scope="scope">
              <el-button type="text" size="mini" @click="applyVoteTarget(scope.row)">填入</el-button>
              <el-button type="text" size="mini" @click="removeLocalVote(scope.row.voteKey)">删除</el-button>
            </template>
          </el-table-column>
        </el-table>
      </el-card>

      <el-form :inline="true" label-width="120px" @submit.native.prevent>
        <el-form-item label="round">
          <el-input-number v-model="voteRound" :min="0" :max="255" :step="1" controls-position="right" />
        </el-form-item>
        <el-form-item label="seq">
          <el-input-number v-model="voteSeq" :min="0" :max="255" :step="1" controls-position="right" />
        </el-form-item>
      </el-form>

      <el-form label-width="120px" @submit.native.prevent>
        <el-form-item label="proposal">
          <el-input v-model="proposal" type="textarea" :rows="2" placeholder="投票提案内容" />
        </el-form-item>
        <el-form-item>
          <el-button type="primary" :loading="isStartingVote" @click="handleStartVote">
            startVote()
          </el-button>
        </el-form-item>
      </el-form>

      <el-divider />

      <el-form :inline="true" label-width="120px" @submit.native.prevent>
        <el-form-item label="notaryId">
          <el-input v-model="notaryId" placeholder="例如 User_0x..." clearable style="width: 420px;" />
        </el-form-item>
        <el-form-item label="agree">
          <el-switch v-model="agree" active-text="赞成" inactive-text="反对" />
        </el-form-item>
        <el-form-item>
          <el-button type="warning" :loading="isVoting" @click="handleVote">
            vote()
          </el-button>
        </el-form-item>
      </el-form>

      <div v-if="voteReceipt" style="margin-top: 12px;">
        <el-alert
          :title="'交易哈希: ' + (voteReceipt.transactionHash || '-') + ' / status: ' + (voteReceipt.status || '-')"
          :type="voteReceipt.status === '0x0' ? 'success' : 'warning'"
          :closable="false"
          :description="voteReceipt.message || voteReceipt.statusMsg || '投票交易已返回回执。'"
          show-icon
        />
        <div v-if="voteReceipt.output" style="margin-top: 10px;">
          <el-tag type="info">output</el-tag>
          <pre style="white-space: pre-wrap; word-break: break-all; margin-top: 6px;">{{ voteReceipt.output }}</pre>
        </div>
      </div>
    </el-card>

    <el-card class="box-card" shadow="hover" style="margin-top: 16px;">
      <div slot="header" class="clearfix">
        <span>通过链上地址查询是否被封禁</span>
      </div>

      <el-form :inline="true" label-width="120px" @submit.native.prevent>
        <el-form-item label="address">
          <el-input v-model="checkAddress" placeholder="要检查的地址" clearable style="width: 420px;" />
        </el-form-item>
        <el-form-item>
          <el-button type="success" :loading="isCheckingBanned" @click="handleCheckBanned">
            banned()
          </el-button>
        </el-form-item>
        <el-form-item label="result">
          <el-tag :type="bannedValue ? 'danger' : 'success'" size="medium">{{ bannedDisplay }}</el-tag>
        </el-form-item>
      </el-form>
    </el-card>
  </div>
</template>

<script>
import {
  reqTrustBanned,
  reqTrustReadTrustValue,
  reqTrustReadTxInfo,
  reqTrustStartVote,
  reqTrustVote
} from '@/api/fisco'
import trustHero from '@/assets/feature-images/trust-vote.svg'

const TRUST_MANAGEMENT_ABI = [
  {
    inputs: [
      { internalType: 'address', name: 'registerAddr', type: 'address' },
      { internalType: 'address', name: 'oracleAddr', type: 'address' }
    ],
    stateMutability: 'nonpayable',
    type: 'constructor'
  },
  {
    inputs: [{ internalType: 'uint256', name: '_id', type: 'uint256' }],
    name: 'readTrustValue',
    outputs: [{ internalType: 'uint256', name: '', type: 'uint256' }],
    stateMutability: 'view',
    type: 'function'
  },
  {
    inputs: [{ internalType: 'uint256', name: 'round', type: 'uint256' }],
    name: 'readTxInfo',
    outputs: [
      { internalType: 'string', name: 'Notary_id', type: 'string' },
      { internalType: 'uint256', name: 'Receivingchain_id', type: 'uint256' },
      { internalType: 'string', name: 'Receiver_address', type: 'string' },
      { internalType: 'string', name: 'total', type: 'string' },
      { internalType: 'uint256', name: 'T_select', type: 'uint256' }
    ],
    stateMutability: 'view',
    type: 'function'
  },
  {
    inputs: [
      { internalType: 'uint8', name: 'round', type: 'uint8' },
      { internalType: 'uint8', name: 'seq', type: 'uint8' },
      { internalType: 'string', name: '_proposal', type: 'string' }
    ],
    name: 'startVote',
    outputs: [],
    stateMutability: 'nonpayable',
    type: 'function'
  },
  {
    inputs: [
      { internalType: 'uint8', name: 'round', type: 'uint8' },
      { internalType: 'uint8', name: 'seq', type: 'uint8' },
      { internalType: 'string', name: '_notaryid', type: 'string' },
      { internalType: 'bool', name: 'agree', type: 'bool' }
    ],
    name: 'vote',
    outputs: [],
    stateMutability: 'nonpayable',
    type: 'function'
  },
  {
    inputs: [{ internalType: 'address', name: '', type: 'address' }],
    name: 'banned',
    outputs: [{ internalType: 'bool', name: '', type: 'bool' }],
    stateMutability: 'view',
    type: 'function'
  }
]

const DEFAULT_TRUST_MANAGEMENT_CONTRACT_ADDRESS = '0x542889cb16859f0a2f94e3a1b0ae06d8c43baa34'

export default {
  name: 'TrustManagementContract',
  data() {
    return {
      heroImg: trustHero,
      userAddress: '',
      contractAddress: DEFAULT_TRUST_MANAGEMENT_CONTRACT_ADDRESS,
      contractAbi: TRUST_MANAGEMENT_ABI,

      trustId: 0,
      isReadingTrust: false,
      trustValue: null,

      txRound: 0,
      isReadingTxInfo: false,
      txInfo: null,

      voteRound: 0,
      voteSeq: 0,
      proposal: '',
      isStartingVote: false,

      // local votes (recorded on startVote)
      localVotes: [],

      notaryId: '',
      agree: true,
      isVoting: false,
      voteReceipt: null,

      checkAddress: '',
      isCheckingBanned: false,
      bannedValue: null
    }
  },
  computed: {
    trustValueDisplay() {
      if (this.trustValue === null || this.trustValue === undefined) return '暂无数据'
      return String(this.trustValue)
    },
    bannedDisplay() {
      if (this.bannedValue === null || this.bannedValue === undefined) return '暂无数据'
      return this.bannedValue ? '已封禁' : '未封禁'
    }
  },
  watch: {
    contractAddress() {
      this.loadLocalVotes()
    }
  },
  created() {
    const addr = this.$store.getters.userAddress
    if (addr && !this.userAddress) this.userAddress = addr
    this.loadLocalVotes()
  },
  methods: {
    ensureBaseParams() {
      if (!this.userAddress) {
        this.$message.warning('请先填写 User 地址')
        return false
      }
      if (!this.contractAddress) {
        this.$message.warning('请先填写合约地址')
        return false
      }
      return true
    },
    normalizeAddress(addr) {
      return String(addr || '').trim().toLowerCase()
    },
    getLocalVotesStorageKey() {
      const caddr = this.normalizeAddress(this.contractAddress)
      return `trust_management_votes:${caddr || 'unknown'}`
    },
    loadLocalVotes() {
      try {
        const raw = localStorage.getItem(this.getLocalVotesStorageKey())
        const list = raw ? JSON.parse(raw) : []
        this.localVotes = Array.isArray(list) ? list : []
      } catch (e) {
        this.localVotes = []
      }
    },
    persistLocalVotes() {
      localStorage.setItem(this.getLocalVotesStorageKey(), JSON.stringify(this.localVotes || []))
    },
    upsertLocalVote(vote) {
      const idx = (this.localVotes || []).findIndex(v => v.voteKey === vote.voteKey)
      if (idx >= 0) {
        this.$set(this.localVotes, idx, { ...this.localVotes[idx], ...vote })
      } else {
        this.localVotes.unshift(vote)
      }
      this.persistLocalVotes()
    },
    async clearLocalVotes() {
      try {
        await this.$confirm('确认清空当前合约地址下的投票记录吗？（仅清前端本地记录）', '提示', {
          confirmButtonText: '清空',
          cancelButtonText: '取消',
          type: 'warning'
        })
      } catch (e) {
        return
      }
      this.localVotes = []
      this.persistLocalVotes()
      this.$message.success('已清空')
    },
    removeLocalVote(voteKey) {
      this.localVotes = (this.localVotes || []).filter(v => v.voteKey !== voteKey)
      this.persistLocalVotes()
    },
    applyVoteTarget(row) {
      this.voteRound = row.round
      this.voteSeq = row.seq
      if (!this.proposal && row.proposal) this.proposal = row.proposal
      this.$message.success(`已填入 round=${row.round}, seq=${row.seq}`)
    },
    async handleReadTrustValue() {
      if (!this.ensureBaseParams()) return
      this.isReadingTrust = true
      this.trustValue = null
      try {
        const response = await reqTrustReadTrustValue({
          userAddress: this.userAddress,
          contractAddress: this.contractAddress,
          id: String(this.trustId),
          abi: this.contractAbi
        })
        if (Array.isArray(response) && response.length) {
          this.trustValue = response[0]
        } else {
          this.trustValue = response
        }
        this.$message.success('查询成功！')
      } catch (e) {
        // eslint-disable-next-line no-console
        console.error(e)
        this.$message.error('查询失败')
      } finally {
        this.isReadingTrust = false
      }
    },
    async handleReadTxInfo() {
      if (!this.ensureBaseParams()) return
      this.isReadingTxInfo = true
      this.txInfo = null
      try {
        const response = await reqTrustReadTxInfo({
          userAddress: this.userAddress,
          contractAddress: this.contractAddress,
          round: String(this.txRound),
          abi: this.contractAbi
        })
        if (Array.isArray(response) && response.length >= 5) {
          this.txInfo = {
            Notary_id: response[0],
            Receivingchain_id: response[1],
            Receiver_address: response[2],
            total: response[3],
            T_select: response[4]
          }
        } else if (response && typeof response === 'object') {
          this.txInfo = response
        } else {
          this.txInfo = { raw: response }
        }
        this.$message.success('查询成功！')
      } catch (e) {
        // eslint-disable-next-line no-console
        console.error(e)
        this.$message.error('查询失败')
      } finally {
        this.isReadingTxInfo = false
      }
    },
    async handleStartVote() {
      if (!this.ensureBaseParams()) return
      if (!this.proposal) {
        this.$message.warning('请填写 proposal')
        return
      }
      this.isStartingVote = true
      try {
        const response = await reqTrustStartVote({
          userAddress: this.userAddress,
          contractAddress: this.contractAddress,
          round: String(this.voteRound),
          seq: String(this.voteSeq),
          proposal: this.proposal,
          abi: this.contractAbi
        })
        if (response && response.status === '0x0') {
          this.$message.success('发起投票成功！')
          this.upsertLocalVote({
            voteKey: `${Number(this.voteRound)}-${Number(this.voteSeq)}`,
            creator: this.userAddress,
            round: Number(this.voteRound),
            seq: Number(this.voteSeq),
            proposal: this.proposal,
            txHash: response.transactionHash || '',
            createdAt: new Date().toISOString()
          })
        } else {
          this.$message.warning('交易已返回，但状态可能异常，请检查回执/控制台')
          // eslint-disable-next-line no-console
          console.warn('TrustManagement.startVote 响应:', response)
        }
      } catch (e) {
        // eslint-disable-next-line no-console
        console.error(e)
        this.$message.error('发起投票失败')
      } finally {
        this.isStartingVote = false
      }
    },
    async handleVote() {
      if (!this.ensureBaseParams()) return
      if (!this.notaryId) {
        this.$message.warning('请填写 notaryId')
        return
      }
      this.isVoting = true
      this.voteReceipt = null
      try {
        const response = await reqTrustVote({
          userAddress: this.userAddress,
          contractAddress: this.contractAddress,
          round: String(this.voteRound),
          seq: String(this.voteSeq),
          notaryId: this.notaryId,
          agree: this.agree,
          abi: this.contractAbi
        })
        this.voteReceipt = response
        if (response && response.status === '0x0') {
          this.$message.success('投票成功！')
        } else {
          this.$message.warning('交易已返回，但状态可能异常，请检查回执/控制台')
          // eslint-disable-next-line no-console
          console.warn('TrustManagement.vote 响应:', response)
        }
      } catch (e) {
        // eslint-disable-next-line no-console
        console.error(e)
        this.$message.error('投票失败')
      } finally {
        this.isVoting = false
      }
    },
    async handleCheckBanned() {
      if (!this.ensureBaseParams()) return
      if (!this.checkAddress) {
        this.$message.warning('请填写 address')
        return
      }
      this.isCheckingBanned = true
      this.bannedValue = null
      try {
        const response = await reqTrustBanned({
          userAddress: this.userAddress,
          contractAddress: this.contractAddress,
          address: this.checkAddress,
          abi: this.contractAbi
        })
        if (Array.isArray(response) && response.length) {
          this.bannedValue = response[0] === true || response[0] === 'true'
        } else {
          this.bannedValue = response === true || response === 'true'
        }
        this.$message.success('查询成功！')
      } catch (e) {
        // eslint-disable-next-line no-console
        console.error(e)
        this.$message.error('查询失败')
      } finally {
        this.isCheckingBanned = false
      }
    }
  }
}
</script>

<style scoped>
.feature-hero {
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: 16px;
  padding: 14px 16px;
  border-radius: 10px;
  margin-bottom: 16px;
  background: linear-gradient(135deg, #1f2d3d 0%, #4c1d95 45%, #ef4444 100%);
  color: #fff;
}
.feature-hero__left {
  min-width: 0;
}
.feature-hero__title {
  font-size: 18px;
  font-weight: 700;
  letter-spacing: 0.4px;
}
.feature-hero__subtitle {
  margin-top: 6px;
  font-size: 13px;
  opacity: 0.9;
}
.feature-hero__img {
  width: 180px;
  max-width: 40%;
  height: auto;
  border-radius: 10px;
  opacity: 0.95;
  filter: drop-shadow(0 10px 20px rgba(0, 0, 0, 0.25));
}
.kv-table {
  border: 1px solid #ebeef5;
  border-radius: 4px;
  overflow: hidden;
}
.kv-row {
  display: flex;
  border-top: 1px solid #ebeef5;
}
.kv-row:first-child {
  border-top: none;
}
.kv-key {
  width: 200px;
  padding: 10px 12px;
  background: #fafafa;
  color: #606266;
  font-size: 12px;
  line-height: 1.4;
  border-right: 1px solid #ebeef5;
}
.kv-val {
  flex: 1;
  padding: 10px 12px;
  min-width: 0;
}
.mono {
  font-family: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, 'Liberation Mono', 'Courier New', monospace;
  font-size: 12px;
  word-break: break-all;
}
.selectable {
  user-select: text;
}
</style>
