<template>
  <div class="app-container">
    <div class="hero">
      <div class="hero-left">
        <div class="hero-title">去中心化且隐私保护的跨链公证人可信选择软件</div>
        <div class="hero-subtitle">区块链数据信息</div>
      </div>
      <img class="hero-img" :src="heroImg" alt="dashboard">
    </div>

    <el-card class="box-card" shadow="hover" style="margin-bottom: 20px;">
      <div slot="header" class="clearfix">
        <span>链上监控</span>
        <el-button
          style="float: right; padding: 3px 0"
          type="text"
          :loading="isOverviewLoading"
          @click="fetchOverview"
        >刷新</el-button>
      </div>

      <el-form :inline="true" @submit.native.prevent>
        <el-form-item label="Group ID:">
          <el-input-number v-model="groupId" :min="1" :max="9999" controls-position="right" />
        </el-form-item>
        <el-form-item>
          <el-button type="primary" :loading="isOverviewLoading" @click="fetchOverview">获取最新高度</el-button>
        </el-form-item>
      </el-form>

      <el-row :gutter="20" style="margin-top: 10px;">
        <el-col :xs="24" :sm="12">
          <div class="metric">
            <div class="metric-label">区块高度</div>
            <div class="metric-value">
              <el-tag type="success" size="medium">{{ blockHeightText }}</el-tag>
              <el-button
                style="margin-left: 10px;"
                size="mini"
                :disabled="blockHeight == null"
                :loading="isBlockLoading"
                @click="queryLatestBlock"
              >查看最新区块</el-button>
            </div>
          </div>
        </el-col>
        <el-col :xs="24" :sm="12">
          <div class="metric">
            <div class="metric-label">交易高度 (累计交易数)</div>
            <div class="metric-value">
              <el-tag type="info" size="medium">{{ txHeightText }}</el-tag>
            </div>
          </div>
        </el-col>
      </el-row>
    </el-card>

    <el-card class="box-card" shadow="hover" style="margin-bottom: 20px;">
      <div slot="header" class="clearfix">
        <span>按区块高度查询区块信息</span>
      </div>

      <el-form :inline="true" @submit.native.prevent>
        <el-form-item label="区块高度:">
          <el-input
            v-model.trim="queryBlockNumber"
            placeholder="例如：100"
            clearable
            style="width: 260px;"
            @keyup.enter.native="queryBlockByHeight"
          />
        </el-form-item>
        <el-form-item>
          <el-button type="primary" :loading="isBlockLoading" @click="queryBlockByHeight">查询</el-button>
        </el-form-item>
        <el-form-item>
          <el-button :disabled="blockHeight == null" :loading="isBlockLoading" @click="queryLatestBlock">查询最新</el-button>
        </el-form-item>
      </el-form>

      <div v-if="blockInfo" style="margin-top: 10px;">
        <div class="kv-table">
          <div class="kv-row">
            <div class="kv-key">number</div>
            <div class="kv-val"><span class="mono selectable">{{ getBlockField(blockInfo, 'number') }}</span></div>
          </div>
          <div class="kv-row">
            <div class="kv-key">hash</div>
            <div class="kv-val"><span class="mono selectable">{{ getBlockField(blockInfo, 'hash') }}</span></div>
          </div>
          <div class="kv-row">
            <div class="kv-key">parentHash</div>
            <div class="kv-val"><span class="mono selectable">{{ getBlockField(blockInfo, 'parentHash') }}</span></div>
          </div>
          <div class="kv-row">
            <div class="kv-key">timestamp</div>
            <div class="kv-val"><span class="mono selectable">{{ getBlockField(blockInfo, 'timestamp') }}</span></div>
          </div>
          <div class="kv-row">
            <div class="kv-key">transactions</div>
            <div class="kv-val"><span class="mono selectable">{{ getTxCount(blockInfo) }}</span></div>
          </div>
          <div class="kv-row">
            <div class="kv-key">sealer</div>
            <div class="kv-val"><span class="mono selectable">{{ getBlockField(blockInfo, 'sealer') }}</span></div>
          </div>
        </div>

        <el-collapse style="margin-top: 10px;">
          <el-collapse-item title="查看原始 JSON" name="raw">
            <pre class="json-pre">{{ prettyJson(blockInfo) }}</pre>
          </el-collapse-item>
        </el-collapse>
      </div>
    </el-card>

    <el-card class="box-card" shadow="hover">
      <div slot="header" class="clearfix">
        <span>按 Tx Hash 查询交易信息</span>
      </div>

      <el-form :inline="true" @submit.native.prevent>
        <el-form-item label="Tx Hash:">
          <el-input
            v-model.trim="txHashInput"
            placeholder="0x + 64位十六进制"
            clearable
            style="width: 420px;"
            @keyup.enter.native="queryTxByHash"
          />
        </el-form-item>
        <el-form-item>
          <el-button type="primary" :loading="isTxLoading" @click="queryTxByHash">查询</el-button>
        </el-form-item>
        <el-form-item>
          <el-button :disabled="!txHashInput" :loading="isTxLoading" @click="clearTxResult">清空</el-button>
        </el-form-item>
      </el-form>

      <div v-if="txInfo || txReceipt" style="margin-top: 10px;">
        <el-alert
          v-if="txReceipt && getBlockField(txReceipt, 'status') !== '-'"
          :title="formatReceiptStatus(txReceipt.status)"
          :type="isReceiptOk(txReceipt.status) ? 'success' : 'warning'"
          show-icon
          :closable="false"
          style="margin-bottom: 10px;"
        />

        <el-row :gutter="20">
          <el-col :xs="24" :sm="12">
            <div>
              <div class="kv-title">交易信息</div>
              <div class="kv-table">
                <div class="kv-row">
                  <div class="kv-key">hash</div>
                  <div class="kv-val"><span class="mono selectable">{{ getBlockField(txInfo, 'hash') }}</span></div>
                </div>
                <div class="kv-row">
                  <div class="kv-key">from</div>
                  <div class="kv-val"><span class="mono selectable">{{ getBlockField(txInfo, 'from') }}</span></div>
                </div>
                <div class="kv-row">
                  <div class="kv-key">to</div>
                  <div class="kv-val"><span class="mono selectable">{{ getBlockField(txInfo, 'to') }}</span></div>
                </div>
                <div class="kv-row">
                  <div class="kv-key">blockNumber</div>
                  <div class="kv-val"><span class="mono selectable">{{ getBlockField(txInfo, 'blockNumber') }}</span></div>
                </div>
                <div class="kv-row">
                  <div class="kv-key">transactionIndex</div>
                  <div class="kv-val"><span class="mono selectable">{{ getBlockField(txInfo, 'transactionIndex') }}</span></div>
                </div>
                <div class="kv-row">
                  <div class="kv-key">input</div>
                  <div class="kv-val"><span class="mono selectable">{{ getBlockField(txInfo, 'input') }}</span></div>
                </div>
              </div>
            </div>
          </el-col>
          <el-col :xs="24" :sm="12">
            <div>
              <div class="kv-title">回执信息</div>
              <div class="kv-table">
                <div class="kv-row">
                  <div class="kv-key">status</div>
                  <div class="kv-val"><span class="mono selectable">{{ getBlockField(txReceipt, 'status') }}</span></div>
                </div>
                <div class="kv-row">
                  <div class="kv-key">blockNumber</div>
                  <div class="kv-val"><span class="mono selectable">{{ getBlockField(txReceipt, 'blockNumber') }}</span></div>
                </div>
                <div class="kv-row">
                  <div class="kv-key">gasUsed</div>
                  <div class="kv-val"><span class="mono selectable">{{ getBlockField(txReceipt, 'gasUsed') }}</span></div>
                </div>
                <div class="kv-row">
                  <div class="kv-key">contractAddress</div>
                  <div class="kv-val"><span class="mono selectable">{{ getBlockField(txReceipt, 'contractAddress') }}</span></div>
                </div>
                <div class="kv-row">
                  <div class="kv-key">output</div>
                  <div class="kv-val"><span class="mono selectable">{{ getBlockField(txReceipt, 'output') }}</span></div>
                </div>
                <div class="kv-row">
                  <div class="kv-key">logs</div>
                  <div class="kv-val"><span class="mono selectable">{{ Array.isArray(txReceipt && txReceipt.logs) ? txReceipt.logs.length : '-' }}</span></div>
                </div>
              </div>
            </div>
          </el-col>
        </el-row>

        <el-collapse style="margin-top: 10px;">
          <el-collapse-item v-if="txInfo" title="查看交易原始 JSON" name="txRaw">
            <pre class="json-pre">{{ prettyJson(txInfo) }}</pre>
          </el-collapse-item>
          <el-collapse-item v-if="txReceipt" title="查看回执原始 JSON" name="receiptRaw">
            <pre class="json-pre">{{ prettyJson(txReceipt) }}</pre>
          </el-collapse-item>
        </el-collapse>
      </div>
    </el-card>

  </div>
</template>

<script>
import {
  reqGetBlockByNumber,
  reqGetBlockNumber,
  reqGetTransactionByHashCompat,
  reqGetTransactionReceipt,
  reqGetTransactionTotal
} from '@/api/webase-monitor'
import dashboardHero from '@/assets/feature-images/dashboard-monitor.svg'

export default {
  name: 'Dashboard',
  data() {
    return {
      heroImg: dashboardHero,
      // overview
      groupId: 1,
      isOverviewLoading: false,
      blockHeight: null,
      txHeight: null,

      // block query
      queryBlockNumber: '',
      isBlockLoading: false,
      blockInfo: null,

      // tx query
      txHashInput: '',
      isTxLoading: false,
      txInfo: null,
      txReceipt: null
    }
  },
  computed: {
    blockHeightText() {
      return this.blockHeight == null ? '暂无' : String(this.blockHeight)
    },
    txHeightText() {
      return this.txHeight == null ? '暂无' : String(this.txHeight)
    }
  },
  created() {
    this.fetchOverview()
  },
  methods: {
    async fetchOverview() {
      this.isOverviewLoading = true
      try {
        const [blockNumberResp, txTotalResp] = await Promise.all([
          reqGetBlockNumber(this.groupId),
          reqGetTransactionTotal(this.groupId)
        ])

        // blockNumberResp 可能是数字/字符串，也可能是 { blockNumber: ... }
        const bn = this.normalizeNumber(blockNumberResp && blockNumberResp.blockNumber != null ? blockNumberResp.blockNumber : blockNumberResp)
        if (bn != null) this.blockHeight = bn

        // txTotalResp 典型结构：{ txSum: 1000, blockNumber: 200 }
        const txSum = txTotalResp && (txTotalResp.txSum != null ? txTotalResp.txSum : txTotalResp.transactionTotal)
        const tx = this.normalizeNumber(txSum)
        if (tx != null) this.txHeight = tx

        // 兼容：如果 transaction-total 里也带 blockNumber，就顺便刷新
        const bn2 = this.normalizeNumber(txTotalResp && txTotalResp.blockNumber)
        if (bn2 != null) this.blockHeight = bn2
      } catch (e) {
        console.error('获取链上高度失败:', e)
        this.$message.error('获取链上高度失败，请检查 WeBASE-Front 代理配置')
      } finally {
        this.isOverviewLoading = false
      }
    },

    async queryBlockByHeight() {
      const height = this.normalizeNumber(this.queryBlockNumber)
      if (height == null || height < 0) {
        this.$message.warning('请输入合法的区块高度')
        return
      }
      await this.fetchBlockByNumber(height)
    },

    async queryLatestBlock() {
      if (this.blockHeight == null) {
        await this.fetchOverview()
      }
      if (this.blockHeight == null) {
        this.$message.warning('暂无最新区块高度')
        return
      }
      await this.fetchBlockByNumber(this.blockHeight)
    },

    async fetchBlockByNumber(height) {
      this.isBlockLoading = true
      this.blockInfo = null
      try {
        const resp = await reqGetBlockByNumber(this.groupId, height)
        this.blockInfo = resp
        this.$message.success('区块查询成功')
      } catch (e) {
        console.error('查询区块失败:', e)
        this.$message.error('查询区块失败')
      } finally {
        this.isBlockLoading = false
      }
    },

    async queryTxByHash() {
      const txHash = (this.txHashInput || '').trim()
      if (!/^0x[0-9a-fA-F]{64}$/.test(txHash)) {
        this.$message.warning('请输入合法的 Tx Hash（0x + 64位十六进制）')
        return
      }

      this.isTxLoading = true
      this.txInfo = null
      this.txReceipt = null
      try {
        const [txRes, receiptRes] = await Promise.allSettled([
          reqGetTransactionByHashCompat(this.groupId, txHash, { silent: true }),
          reqGetTransactionReceipt(this.groupId, txHash, { silent: true })
        ])

        if (txRes.status === 'fulfilled') this.txInfo = txRes.value
        if (receiptRes.status === 'fulfilled') this.txReceipt = receiptRes.value

        if (!this.txInfo && !this.txReceipt) {
          const msg = this.pickWeBaseErrorMessage(txRes, receiptRes) || '未查询到交易信息，请确认 Tx Hash / Group ID 是否正确'
          this.$message.error(msg)
          return
        }

        if (this.txInfo && this.txReceipt) {
          this.$message.success('交易查询成功')
        } else {
          const msg = this.pickWeBaseErrorMessage(txRes, receiptRes) || '仅查询到部分信息（交易详情/回执其一）'
          this.$message.warning(msg)
        }
      } catch (e) {
        console.error('查询交易失败:', e)
        this.$message.error('查询交易失败')
      } finally {
        this.isTxLoading = false
      }
    },

    clearTxResult() {
      this.txHashInput = ''
      this.txInfo = null
      this.txReceipt = null
    },

    isReceiptOk(status) {
      if (status == null) return false
      const s = String(status).trim().toLowerCase()
      // FISCO BCOS 常见：0x0 / 0x00 表示成功；Ethereum 常见：0x1 表示成功
      return s === '0x0' || s === '0x00' || s === '0x1' || s === '0x01' || s === '0'
    },

    formatReceiptStatus(status) {
      if (status == null) return '回执状态：未知'
      return this.isReceiptOk(status) ? `回执状态：成功 (${status})` : `回执状态：失败/异常 (${status})`
    },

    pickWeBaseErrorMessage(txRes, receiptRes) {
      const parts = []

      if (txRes && txRes.status === 'rejected') {
        const status = txRes.reason && txRes.reason.response && txRes.reason.response.status
        if (status === 404) {
          parts.push('交易详情接口返回 404（可能 WeBASE-Front 版本不支持 transactionByHash）')
        } else {
          parts.push(`交易详情获取失败${status ? `（HTTP ${status}）` : ''}`)
        }
      }

      if (receiptRes && receiptRes.status === 'rejected') {
        const status = receiptRes.reason && receiptRes.reason.response && receiptRes.reason.response.status
        parts.push(`回执获取失败${status ? `（HTTP ${status}）` : ''}`)
      }

      return parts.length ? parts.join('；') : ''
    },

    normalizeNumber(val) {
      if (val === '' || val == null) return null
      if (typeof val === 'number' && Number.isFinite(val)) return val
      if (typeof val === 'string') {
        const trimmed = val.trim()
        if (!trimmed) return null
        // hex string like 0x10
        if (/^0x[0-9a-fA-F]+$/.test(trimmed)) return parseInt(trimmed, 16)
        if (/^\d+$/.test(trimmed)) return parseInt(trimmed, 10)
      }
      return null
    },

    getBlockField(block, key) {
      if (!block) return '-'
      const v = block[key]
      if (v == null || v === '') return '-'
      if (typeof v === 'object') return this.prettyJson(v)
      return String(v)
    },

    getTxCount(block) {
      if (!block) return '-'
      const txs = block.transactions
      if (Array.isArray(txs)) return txs.length
      // 有些版本返回 txHashes
      const hashes = block.transactionHashes
      if (Array.isArray(hashes)) return hashes.length
      return '-'
    },

    prettyJson(obj) {
      try {
        return JSON.stringify(obj, null, 2)
      } catch (e) {
        return String(obj)
      }
    }
  }
}
</script>

<style scoped>
.hero {
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: 16px;
  padding: 14px 16px;
  margin-bottom: 16px;
  border-radius: 10px;
  background: linear-gradient(135deg, #1f2d3d 0%, #223a5a 50%, #2f5aa6 100%);
  color: #fff;
}
.hero-left {
  min-width: 0;
}
.hero-title {
  font-size: 20px;
  font-weight: 700;
  letter-spacing: 0.5px;
}
.hero-subtitle {
  margin-top: 6px;
  font-size: 14px;
  opacity: 0.9;
}
.hero-img {
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
.json-pre {
  margin: 0;
  padding: 10px;
  background: #0b1020;
  color: #e6edf3;
  border-radius: 4px;
  overflow: auto;
  font-size: 12px;
  line-height: 1.5;
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
  width: 180px;
  padding: 10px 12px;
  background: #fafafa;
  color: #606266;
  font-size: 12px;
  line-height: 1.4;
  border-right: 1px solid #ebeef5;
}
.kv-title {
  font-size: 13px;
  font-weight: 700;
  color: #303133;
  margin: 4px 0 8px;
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
