<template>
  <div class="app-container">
    <div class="feature-hero">
      <div class="feature-hero__left">
        <div class="feature-hero__title">Count</div>
        <div class="feature-hero__subtitle">跨链统计与上报</div>
      </div>
      <img class="feature-hero__img" :src="heroImg" alt="count">
    </div>

    <el-card class="box-card" shadow="hover">
      <div slot="header" class="clearfix">
        <span>Count 合约 - add</span>
      </div>

      <el-form :inline="true" label-width="120px" @submit.native.prevent>
        <el-form-item label="User 地址">
          <el-input v-model="userAddress" placeholder="WeBASE 用户地址" clearable style="width: 420px;" />
        </el-form-item>
        <el-form-item label="合约地址">
          <el-input v-model="contractAddress" placeholder="Count 合约地址" clearable style="width: 420px;" />
        </el-form-item>
      </el-form>

      <el-form label-width="140px" @submit.native.prevent>
        <el-form-item label="资产 i (0-2)">
          <el-input-number v-model="form.assetIndex" :min="0" :max="2" :step="1" controls-position="right" />
        </el-form-item>
        <el-form-item label="接收链 r (0-2)">
          <el-input-number v-model="form.receivingAssetIndex" :min="0" :max="2" :step="1" controls-position="right" />
        </el-form-item>
        <el-form-item label="Receiver 地址">
          <el-input v-model="form.receiverAddress" placeholder="字符串地址（不同链格式可不同）" clearable />
        </el-form-item>
        <el-form-item label="value">
          <el-input-number v-model="form.value" :min="1" :step="1" controls-position="right" />
        </el-form-item>
        <el-form-item>
          <el-button type="primary" :loading="isAdding" @click="handleAdd">
            add()
          </el-button>
        </el-form-item>
      </el-form>

      <div v-if="addReceipt" style="margin-top: 16px;">
        <el-alert
          :title="'交易哈希: ' + (addReceipt.transactionHash || '-')"
          type="success"
          :closable="false"
          description="交易已提交并返回回执（若 status=0x0 通常表示成功）。"
          show-icon
        />
        <div v-if="addReceipt.output" style="margin-top: 10px;">
          <el-tag type="info">output</el-tag>
          <pre style="white-space: pre-wrap; word-break: break-all; margin-top: 6px;">{{ addReceipt.output }}</pre>
        </div>
      </div>
    </el-card>

    <el-card class="box-card" shadow="hover" style="margin-top: 16px;">
      <div slot="header" class="clearfix">
        <span>Count 合约 - 查询</span>
      </div>

      <el-alert
        title="时间窗口说明"
        type="info"
        :closable="false"
        show-icon
        description="Count 合约内部使用 40 秒窗口（T=40000ms）。倒计时从“第一笔 add() 成功后”开始；到期后，下一笔 add() 才会触发结算并开启新窗口（此时重置倒计时）。"
        style="margin-bottom: 12px;"
      />

      <el-card shadow="never" style="border: 1px solid #ebeef5; margin-bottom: 12px;">
        <div slot="header" class="clearfix">
          <span>时间倒计时（估算）</span>
          <el-button
            style="float: right; padding: 3px 0"
            type="text"
            :disabled="!canSyncCountdown"
            :loading="isSyncingCountdown"
            @click="syncCountdownNow"
          >手动同步</el-button>
        </div>

        <el-row :gutter="20">
          <el-col :xs="24" :sm="8">
            <div class="metric">
              <div class="metric-label">链上 StartTime</div>
              <div class="metric-value">
                <el-tag type="info" size="medium">{{ startTimeText }}</el-tag>
              </div>
            </div>
          </el-col>
          <el-col :xs="24" :sm="8">
            <div class="metric">
              <div class="metric-label">剩余时间</div>
              <div class="metric-value">
                <el-tag :type="countdownRemainingMs > 0 ? 'success' : 'warning'" size="medium">{{ countdownRemainingText }}</el-tag>
              </div>
            </div>
          </el-col>
          <el-col :xs="24" :sm="8" style="margin-top: 8px;">
            <div class="metric">
              <div class="metric-label">窗口长度</div>
              <div class="metric-value">
                <el-tag type="primary" size="medium">{{ timeWindowText }}</el-tag>
              </div>
            </div>
          </el-col>
        </el-row>

        <el-progress
          :percentage="countdownPercent"
          :status="countdownRemainingMs > 0 ? 'success' : 'exception'"
          :stroke-width="10"
          style="margin-top: 10px;"
        />

        <div style="margin-top: 8px; color: #909399; font-size: 12px;">
          <span v-if="!countdownActivated">未开始：等待第一笔 add() 成功后启动倒计时。</span>
          <span v-else>同步时间：{{ lastSyncText }}</span>
        </div>
      </el-card>

      <el-form :inline="true" label-width="120px" @submit.native.prevent>
        <el-form-item>
          <el-button type="success" :loading="isQueryingRound" @click="handleRound">
            round()
          </el-button>
        </el-form-item>
        <el-form-item label="当前轮次是">
          <el-tag type="info" size="medium">{{ roundDisplay }}</el-tag>
        </el-form-item>
      </el-form>

      <el-form :inline="true" label-width="130px" @submit.native.prevent>
        <el-form-item>
          <el-button type="success" :loading="isQueryingTotal" @click="handleGetTotal">
            getTotal()
          </el-button>
        </el-form-item>
        <el-form-item label="当前轮次资产总数">
          <el-tag type="info" size="medium">{{ totalDisplay }}</el-tag>
        </el-form-item>
      </el-form>
    </el-card>

    <el-card class="box-card" shadow="hover" style="margin-top: 16px;">
      <div slot="header" class="clearfix">
        <span>通过接收方地址查询其最近所在轮次</span>
      </div>

      <el-form :inline="true" label-width="140px" @submit.native.prevent>
        <el-form-item label="receiver_address">
          <el-input v-model="receiverQueryAddress" placeholder="例如 0x... 或其他链地址字符串" clearable style="width: 420px;" />
        </el-form-item>
        <el-form-item>
          <el-button type="success" :loading="isQueryingReceiverLastRound" @click="handleReceiverLastRound">
            receiverLastRound()
          </el-button>
        </el-form-item>
        <el-form-item label="lastRound">
          <el-tag type="info" size="medium">{{ receiverLastRoundDisplay }}</el-tag>
        </el-form-item>
      </el-form>

      <el-divider />

      <el-form :inline="true" label-width="120px" @submit.native.prevent>
        <el-form-item label="round">
          <el-input-number v-model="eventRoundFrom" :min="0" :step="1" controls-position="right" />
          <span style="margin: 0 8px;">-</span>
          <el-input-number v-model="eventRoundTo" :min="0" :step="1" controls-position="right" />
        </el-form-item>
        <el-form-item>
          <el-button type="primary" :loading="isLoadingEventList" @click="handleFetchAddressToNotary">
            获取已选择出的公证人记录
          </el-button>
        </el-form-item>
        <el-form-item>
          <el-switch v-model="onlyShowNotaryFilled" active-text="仅显示已产生 notary 的轮次" />
        </el-form-item>
      </el-form>

      <el-table :data="eventListDisplay" size="mini" border style="width: 100%; margin-top: 10px;">
        <el-table-column prop="round" label="round" width="80" />
        <el-table-column prop="receiver" label="receiver" min-width="220" show-overflow-tooltip />
        <el-table-column prop="notary" label="notary" min-width="180" show-overflow-tooltip />
        <el-table-column prop="total" label="total" min-width="160" show-overflow-tooltip />
        <el-table-column prop="tSelect" label="T_select" width="120" />
      </el-table>
    </el-card>

  </div>
</template>

<script>
import {
  reqCountAdd,
  reqCountGetTotal,
  reqCountRound,
  reqCountReceiverLastRound,
  reqCountStartTime,
  reqCountTxInfoByRound
} from '@/api/fisco'
import { reqGetBlockByNumber, reqGetBlockNumber } from '@/api/webase-monitor'
import countHero from '@/assets/feature-images/count-analytics.svg'

const COUNT_ABI = [
  { inputs: [{ internalType: 'address', name: 'oracleAddress', type: 'address' }], stateMutability: 'nonpayable', type: 'constructor' },
  {
    inputs: [
      { internalType: 'uint256', name: 'i', type: 'uint256' },
      { internalType: 'uint256', name: 'r', type: 'uint256' },
      { internalType: 'string', name: 'receiver_address', type: 'string' },
      { internalType: 'uint256', name: 'value', type: 'uint256' }
    ],
    name: 'add',
    outputs: [{ internalType: 'bytes32', name: '', type: 'bytes32' }],
    stateMutability: 'nonpayable',
    type: 'function'
  },
  { inputs: [], name: 'getTotal', outputs: [{ internalType: 'uint256[]', name: '', type: 'uint256[]' }], stateMutability: 'view', type: 'function' },
  { inputs: [], name: 'round', outputs: [{ internalType: 'uint256', name: '', type: 'uint256' }], stateMutability: 'view', type: 'function' },
  { inputs: [], name: 'StartTime', outputs: [{ internalType: 'uint256', name: '', type: 'uint256' }], stateMutability: 'view', type: 'function' },
  { inputs: [{ internalType: 'string', name: '', type: 'string' }], name: 'receiverLastRound', outputs: [{ internalType: 'uint256', name: '', type: 'uint256' }], stateMutability: 'view', type: 'function' },
  {
    inputs: [{ internalType: 'uint256', name: '', type: 'uint256' }],
    name: 'Txid_to_Txinfo',
    outputs: [
      { internalType: 'string', name: 'Notary_id', type: 'string' },
      { internalType: 'uint256', name: 'Receivingchain_id', type: 'uint256' },
      { internalType: 'string', name: 'Receiver_address', type: 'string' },
      { internalType: 'string', name: 'total', type: 'string' },
      { internalType: 'uint256', name: 'T_select', type: 'uint256' }
    ],
    stateMutability: 'view',
    type: 'function'
  }
]

const DEFAULT_COUNT_CONTRACT_ADDRESS = '0x3ce7ca0cd23058ea18193dc1f4959f8530441ca6'

export default {
  name: 'CountContract',
  data() {
    return {
      heroImg: countHero,
      groupId: 1,
      userAddress: '',
      contractAddress: DEFAULT_COUNT_CONTRACT_ADDRESS,
      contractAbi: COUNT_ABI,

      timeWindowMs: 40000,
      countdownActivated: false,
      chainStartTimeMs: null,
      syncChainNowMs: null,
      syncLocalNowMs: null,
      localNowMs: Date.now(),
      countdownTickTimer: null,
      isSyncingCountdown: false,

      form: {
        assetIndex: 0,
        receivingAssetIndex: 0,
        receiverAddress: '',
        value: 1
      },

      isAdding: false,
      addReceipt: null,

      isQueryingTotal: false,
      total: null,

      isQueryingRound: false,
      roundValue: null,

      receiverQueryAddress: '',
      isQueryingReceiverLastRound: false,
      receiverLastRoundValue: null,

      eventRoundFrom: 1,
      eventRoundTo: 10,
      onlyShowNotaryFilled: true,
      isLoadingEventList: false,
      eventList: []
    }
  },
  computed: {
    canSyncCountdown() {
      return Boolean(this.userAddress && this.contractAddress && this.countdownActivated)
    },
    timeWindowText() {
      return `${Math.round(this.timeWindowMs / 1000)}s`
    },
    chainNowMs() {
      if (!this.syncChainNowMs || !this.syncLocalNowMs) return null
      return this.syncChainNowMs + (this.localNowMs - this.syncLocalNowMs)
    },
    countdownRemainingMs() {
      if (!this.countdownActivated) return 0
      if (!this.chainStartTimeMs || !this.chainNowMs) return 0
      return Math.max(0, (this.chainStartTimeMs + this.timeWindowMs) - this.chainNowMs)
    },
    countdownRemainingText() {
      if (!this.countdownActivated) return '未开始'
      if (!this.chainStartTimeMs || !this.chainNowMs) return '未同步'
      const ms = this.countdownRemainingMs
      if (ms <= 0) return '已到期'
      const sec = Math.ceil(ms / 1000)
      return `${sec}s`
    },
    countdownPercent() {
      if (!this.countdownActivated || !this.chainStartTimeMs || !this.chainNowMs) return 0
      const used = this.timeWindowMs - this.countdownRemainingMs
      const pct = Math.round((used / this.timeWindowMs) * 100)
      return Math.max(0, Math.min(100, pct))
    },
    startTimeText() {
      if (!this.countdownActivated) return '未开始'
      if (!this.chainStartTimeMs) return '-'
      return new Date(this.chainStartTimeMs).toLocaleString()
    },
    lastSyncText() {
      if (!this.syncLocalNowMs) return '-'
      return new Date(this.syncLocalNowMs).toLocaleString()
    },
    totalDisplay() {
      if (this.total === null || this.total === undefined) return '暂无数据'
      if (Array.isArray(this.total)) return JSON.stringify(this.total)
      return String(this.total)
    },
    roundDisplay() {
      if (this.roundValue === null || this.roundValue === undefined) return '暂无数据'
      return String(this.roundValue)
    },
    receiverLastRoundDisplay() {
      if (this.receiverLastRoundValue === null || this.receiverLastRoundValue === undefined) return '暂无数据'
      return String(this.receiverLastRoundValue)
    },
    eventListDisplay() {
      if (!this.onlyShowNotaryFilled) return this.eventList
      return (this.eventList || []).filter(i => i.notary)
    }
  },
  created() {
    const addr = this.$store.getters.userAddress
    if (addr && !this.userAddress) this.userAddress = addr
  },
  beforeDestroy() {
    this.stopCountdownTick()
  },
  methods: {
    stopCountdownTick() {
      if (this.countdownTickTimer) {
        clearInterval(this.countdownTickTimer)
        this.countdownTickTimer = null
      }
    },
    startCountdownTick() {
      if (this.countdownTickTimer) return
      this.localNowMs = Date.now()
      this.countdownTickTimer = setInterval(() => {
        this.localNowMs = Date.now()
      }, 1000)
    },
    normalizeNumber(val) {
      if (val === '' || val == null) return null
      if (typeof val === 'number' && Number.isFinite(val)) return val
      if (typeof val === 'string') {
        const trimmed = val.trim()
        if (!trimmed) return null
        if (/^0x[0-9a-fA-F]+$/.test(trimmed)) return parseInt(trimmed, 16)
        if (/^\d+$/.test(trimmed)) return parseInt(trimmed, 10)
      }
      return null
    },
    normalizeTimestampMs(v) {
      const n = this.normalizeNumber(v)
      if (n == null) return null
      // 秒级时间戳 < 1e12；毫秒级通常 >= 1e12
      return n < 1e12 ? n * 1000 : n
    },
    async fetchRoundValue({ silent } = {}) {
      if (!this.ensureBaseParams()) return null
      try {
        const response = await reqCountRound({
          userAddress: this.userAddress,
          contractAddress: this.contractAddress,
          abi: this.contractAbi
        })
        const v = Array.isArray(response) && response.length ? response[0] : response
        const n = Number(v)
        return Number.isFinite(n) ? n : null
      } catch (e) {
        if (!silent) this.$message.error('查询 round 失败')
        return null
      }
    },
    async fetchStartTimeMs({ silent } = {}) {
      if (!this.ensureBaseParams()) return null
      try {
        const response = await reqCountStartTime({
          userAddress: this.userAddress,
          contractAddress: this.contractAddress,
          abi: this.contractAbi
        })
        const v = Array.isArray(response) && response.length ? response[0] : response
        return this.normalizeTimestampMs(v)
      } catch (e) {
        if (!silent) this.$message.error('查询 StartTime 失败')
        return null
      }
    },
    async syncCountdownNow() {
      if (!this.userAddress || !this.contractAddress) {
        this.$message.warning('请先填写 User 地址与合约地址')
        return
      }
      this.isSyncingCountdown = true
      try {
        const startTimeMs = await this.fetchStartTimeMs({ silent: true })
        const chainNowMs = await this.fetchChainNowMs({ silent: true })
        if (startTimeMs == null || chainNowMs == null) return
        this.countdownActivated = true
        this.chainStartTimeMs = startTimeMs
        this.syncChainNowMs = chainNowMs
        this.syncLocalNowMs = Date.now()
        this.startCountdownTick()
        this.$message.success('已同步倒计时')
      } finally {
        this.isSyncingCountdown = false
      }
    },
    async fetchChainNowMs({ silent } = {}) {
      try {
        const blockNumberResp = await reqGetBlockNumber(this.groupId)
        const bn = this.normalizeNumber(blockNumberResp && blockNumberResp.blockNumber != null ? blockNumberResp.blockNumber : blockNumberResp)
        if (bn == null) return null
        const block = await reqGetBlockByNumber(this.groupId, bn)
        const ts = block && (block.timestamp != null ? block.timestamp : block.blockTimestamp)
        return this.normalizeTimestampMs(ts)
      } catch (e) {
        if (!silent) this.$message.error('获取链上时间失败')
        return null
      }
    },
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
    async handleReceiverLastRound() {
      if (!this.ensureBaseParams()) return
      if (!this.receiverQueryAddress) {
        this.$message.warning('请填写 receiver_address')
        return
      }
      this.isQueryingReceiverLastRound = true
      this.receiverLastRoundValue = null
      try {
        const response = await reqCountReceiverLastRound({
          userAddress: this.userAddress,
          contractAddress: this.contractAddress,
          receiverAddress: this.receiverQueryAddress,
          abi: this.contractAbi
        })
        if (Array.isArray(response) && response.length) {
          this.receiverLastRoundValue = response[0]
        } else {
          this.receiverLastRoundValue = response
        }
        this.$message.success('查询成功！')
      } catch (e) {
        // eslint-disable-next-line no-console
        console.error(e)
        this.$message.error('查询失败')
      } finally {
        this.isQueryingReceiverLastRound = false
      }
    },
    async handleFetchAddressToNotary() {
      if (!this.ensureBaseParams()) return
      const from = Math.min(this.eventRoundFrom, this.eventRoundTo)
      const to = Math.max(this.eventRoundFrom, this.eventRoundTo)
      const total = to - from + 1
      if (total > 60) {
        this.$message.warning('查询范围过大（>60），请缩小 round 范围')
        return
      }
      this.isLoadingEventList = true
      try {
        const rounds = []
        for (let r = from; r <= to; r++) rounds.push(r)
        const rows = await Promise.all(rounds.map(async r => {
          try {
            const resp = await reqCountTxInfoByRound({
              userAddress: this.userAddress,
              contractAddress: this.contractAddress,
              round: String(r),
              abi: this.contractAbi
            })
            if (!Array.isArray(resp) || resp.length < 5) return null
            return {
              round: r,
              notary: resp[0] || '',
              receiver: resp[2] || '',
              total: resp[3] || '',
              tSelect: resp[4] != null ? String(resp[4]) : ''
            }
          } catch (e) {
            return null
          }
        }))
        this.eventList = rows.filter(Boolean).sort((a, b) => b.round - a.round)
        this.$message.success(`获取完成：${this.eventList.length} 条`)
      } catch (e) {
        // eslint-disable-next-line no-console
        console.error(e)
        this.$message.error('获取失败')
      } finally {
        this.isLoadingEventList = false
      }
    },
    async handleAdd() {
      if (!this.ensureBaseParams()) return
      if (!this.form.receiverAddress) {
        this.$message.warning('请填写 Receiver 地址')
        return
      }

      this.isAdding = true
      this.addReceipt = null
      try {
        const response = await reqCountAdd({
          userAddress: this.userAddress,
          contractAddress: this.contractAddress,
          assetIndex: String(this.form.assetIndex),
          receivingAssetIndex: String(this.form.receivingAssetIndex),
          receiverAddress: this.form.receiverAddress,
          value: String(this.form.value),
          abi: this.contractAbi
        })
        this.addReceipt = response
        if (response && response.status === '0x0') {
          this.$message.success('交易上链成功！')
          // 规则：首笔 add() 后启动倒计时；到期后，下一笔 add() 成功时重置倒计时（用链上 StartTime 同步）
          if (!this.countdownActivated || this.countdownRemainingMs <= 0) {
            await this.syncCountdownNow()
          }
        } else {
          this.$message.warning('交易已返回，但状态可能异常，请检查回执/控制台')
          // eslint-disable-next-line no-console
          console.warn('Count.add 响应:', response)
        }
      } catch (e) {
        // eslint-disable-next-line no-console
        console.error(e)
        this.$message.error('调用失败')
      } finally {
        this.isAdding = false
      }
    },
    async handleGetTotal() {
      if (!this.ensureBaseParams()) return
      this.isQueryingTotal = true
      try {
        const response = await reqCountGetTotal({
          userAddress: this.userAddress,
          contractAddress: this.contractAddress,
          abi: this.contractAbi
        })
        this.total = response
        this.$message.success('查询成功！')
      } catch (e) {
        // eslint-disable-next-line no-console
        console.error(e)
        this.$message.error('查询失败')
      } finally {
        this.isQueryingTotal = false
      }
    },
    async handleRound() {
      if (!this.ensureBaseParams()) return
      this.isQueryingRound = true
      try {
        const r = await this.fetchRoundValue({ silent: true })
        this.roundValue = r
        this.$message.success('查询成功！')
      } catch (e) {
        // eslint-disable-next-line no-console
        console.error(e)
        this.$message.error('查询失败')
      } finally {
        this.isQueryingRound = false
      }
    }
    // reportBan()/reportUnlock() 仅供外部合约调用，前端不提供入口
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
  background: linear-gradient(135deg, #1f2d3d 0%, #134e4a 45%, #10b981 100%);
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
.metric {
  padding: 6px 0;
}
.metric-label {
  color: #606266;
  font-size: 13px;
  margin-bottom: 6px;
}
.metric-value {
  display: flex;
  align-items: center;
  flex-wrap: wrap;
  gap: 6px;
}
</style>
