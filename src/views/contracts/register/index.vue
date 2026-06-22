<template>
  <div class="app-container">
    <div class="feature-hero">
      <div class="feature-hero__left">
        <div class="feature-hero__title">Register</div>
        <div class="feature-hero__subtitle">密文信息上链登记</div>
      </div>
      <img class="feature-hero__img" :src="heroImg" alt="register">
    </div>

    <el-dialog
      title="注册成功"
      :visible.sync="successDialogVisible"
      width="520px"
    >
      <el-result icon="success" title="TruORA 回调已确认" sub-title="result = ok" />
      <div style="margin-top: 10px;">
        <div class="kv-table">
          <div class="kv-row">
            <div class="kv-key">交易哈希 (transactionHash)</div>
            <div class="kv-val">
              <div class="copy-row">
                <span class="mono selectable">{{ (txReceipt && txReceipt.transactionHash) || '-' }}</span>
                <el-button type="text" size="mini" @click="copyText((txReceipt && txReceipt.transactionHash) || '', '已复制 Tx Hash')">复制</el-button>
              </div>
            </div>
          </div>
          <div class="kv-row">
            <div class="kv-key">合约地址 (contractAddress)</div>
            <div class="kv-val">
              <div class="copy-row">
                <span class="mono selectable">{{ contractAddress || '-' }}</span>
                <el-button type="text" size="mini" @click="copyText(contractAddress, '已复制合约地址')">复制</el-button>
              </div>
            </div>
          </div>
          <div class="kv-row">
            <div class="kv-key">User 地址 (user)</div>
            <div class="kv-val">
              <div class="copy-row">
                <span class="mono selectable">{{ userAddress || '-' }}</span>
                <el-button type="text" size="mini" @click="copyText(userAddress, '已复制 User 地址')">复制</el-button>
              </div>
            </div>
          </div>
          <div class="kv-row">
            <div class="kv-key">链上 _id (Notary_id)</div>
            <div class="kv-val">
              <div class="copy-row">
                <span class="mono selectable">{{ registeredIdDisplay }}</span>
                <el-button type="text" size="mini" :disabled="registeredId == null" @click="copyText(registeredIdDisplay, '已复制 _id')">复制</el-button>
              </div>
            </div>
          </div>
        </div>
        <div style="margin-top: 8px; text-align: right;">
          <el-button type="primary" plain size="mini" @click="copyAllSuccessInfo">复制全部</el-button>
        </div>
      </div>
      <span slot="footer" class="dialog-footer">
        <el-button type="primary" @click="successDialogVisible = false">确定</el-button>
      </span>
    </el-dialog>

    <el-card class="box-card" shadow="hover">
      <div slot="header" class="clearfix">
        <span>输入信息注册成为公证人(storeCipherData)</span>
      </div>

      <el-form :inline="true" label-width="120px" @submit.native.prevent>
        <el-form-item label="User 地址">
          <el-input v-model="userAddress" placeholder="WeBASE 用户地址" clearable style="width: 420px;" />
        </el-form-item>
        <el-form-item label="WeBASE 账户">
          <el-select
            v-model="selectedWeBaseAddress"
            placeholder="选择有私钥的账户"
            clearable
            filterable
            style="width: 420px;"
            :loading="isLoadingWeBaseAccounts"
            @change="applySelectedWeBaseAddress"
          >
            <el-option
              v-for="item in weBaseAccounts"
              :key="item.address"
              :label="formatWeBaseAccountLabel(item)"
              :value="item.address"
            />
          </el-select>
          <el-button type="text" :loading="isLoadingWeBaseAccounts" style="margin-left: 8px;" @click="fetchWeBaseAccounts">刷新</el-button>
        </el-form-item>
        <el-form-item label="合约地址">
          <el-input v-model="contractAddress" placeholder="Register 合约地址" clearable style="width: 420px;" />
        </el-form-item>
      </el-form>

      <el-divider />

      <el-form label-width="120px" @submit.native.prevent>
        <el-form-item label="加密（Encode.py）">
          <el-alert
            title="输入 Asset1/2/3，后端自动生成临时密钥并生成 storeCipherData 的 ciphertext/iv/tag/aad/pub"
            type="info"
            :closable="false"
            show-icon
          />
        </el-form-item>
        <el-form-item label="Asset1">
          <el-input v-model.trim="encodeForm.asset1" placeholder="例如 50.000" clearable />
        </el-form-item>
        <el-form-item label="Asset2">
          <el-input v-model.trim="encodeForm.asset2" placeholder="例如 30.000" clearable />
        </el-form-item>
        <el-form-item label="Asset3">
          <el-input v-model.trim="encodeForm.asset3" placeholder="例如 300.000" clearable />
        </el-form-item>
        <el-form-item>
          <el-button type="primary" plain :loading="isEncoding" @click="handleEncode">
            生成密文并填入表单
          </el-button>
          <el-button :disabled="!encodeResult" @click="applyEncodeResult">仅填入</el-button>
          <el-button :disabled="!encodeResult" @click="clearEncodeResult">清空结果</el-button>
        </el-form-item>
      </el-form>

      <div v-if="encodeResult" style="margin-top: 10px;">
        <el-alert
          :title="'plaintexts: ' + (encodeResult.plaintext || '-')"
          type="success"
          :closable="false"
          show-icon
        />
      </div>

      <el-form label-width="120px" @submit.native.prevent>
        <el-form-item label="Notary IP">
          <el-input v-model="form.ip" placeholder="例如 127.0.0.1" clearable />
        </el-form-item>
        <el-form-item label="ciphertext">
          <el-input v-model="form.ciphertext" type="textarea" :rows="2" placeholder="密文" />
        </el-form-item>
        <el-form-item label="iv">
          <el-input v-model="form.iv" placeholder="iv" clearable />
        </el-form-item>
        <el-form-item label="tag">
          <el-input v-model="form.tag" placeholder="tag" clearable />
        </el-form-item>
        <el-form-item label="aad">
          <el-input v-model="form.aad" placeholder="aad" clearable />
        </el-form-item>
        <el-form-item label="pub">
          <el-input v-model="form.pub" placeholder="pub" clearable />
        </el-form-item>
        <el-form-item>
          <el-button type="primary" :loading="isTransacting" @click="handleStoreCipherData">
            写入区块链
          </el-button>
        </el-form-item>
      </el-form>

      <div v-if="txReceipt" style="margin-top: 16px;">
        <el-alert
          :title="'交易哈希: ' + (txReceipt.transactionHash || '-')"
          type="success"
          :closable="false"
          description="交易已提交并返回回执（若 status=0x0 通常表示成功）。"
          show-icon
        />
        <div v-if="txReceipt.output" style="margin-top: 10px;">
          <el-tag type="info">output</el-tag>
          <pre style="white-space: pre-wrap; word-break: break-all; margin-top: 6px;">{{ txReceipt.output }}</pre>
        </div>
      </div>

      <div v-if="isWaitingCallback" style="margin-top: 12px;">
        <el-alert
          title="等待 TruORA 回调中…"
          type="warning"
          :closable="false"
          show-icon
        >
          <div slot="description">
            <span><i class="el-icon-loading" /> 正在轮询链上回调结果</span>
            <el-tag v-if="callbackResult" type="info" size="mini" style="margin-left: 8px;">result: {{ callbackResult }}</el-tag>
            <el-button type="text" style="margin-left: 8px;" @click="pollCallbackOnce">手动刷新</el-button>
            <el-button type="text" style="margin-left: 8px;" @click="stopCallbackPolling">停止</el-button>
          </div>
        </el-alert>
      </div>
    </el-card>

    <el-card class="box-card" shadow="hover" style="margin-top: 16px;">
      <div slot="header" class="clearfix">
        <span>通过公证人ID查询信誉值</span>
      </div>

      <el-form :inline="true" label-width="140px" @submit.native.prevent>
        <el-form-item label="Trust id">
          <el-input-number v-model="trustId" :min="0" :step="1" controls-position="right" />
        </el-form-item>
        <el-form-item>
          <el-button type="success" :loading="isQueryingTrust" @click="handleGetTrustValuebyId">
            getTrustValuebyId()
          </el-button>
        </el-form-item>
        <el-form-item label="Trust value">
          <el-tag type="info" size="medium">{{ trustValueDisplay }}</el-tag>
        </el-form-item>
      </el-form>
    </el-card>

    <el-card class="box-card" shadow="hover" style="margin-top: 16px;">
      <div slot="header" class="clearfix">
        <span>已注册公证人列表</span>
        <el-button
          style="float: right; padding: 3px 0"
          type="text"
          :loading="isLoadingNotaries"
          @click="handleFetchNotaries"
        >刷新</el-button>
      </div>

      <el-form :inline="true" label-width="120px" @submit.native.prevent>
        <el-form-item label="查询范围(id)">
          <el-input-number v-model="notaryIdFrom" :min="0" :step="1" controls-position="right" />
          <span style="margin: 0 8px;">-</span>
          <el-input-number v-model="notaryIdTo" :min="0" :step="1" controls-position="right" />
        </el-form-item>
        <el-form-item>
          <el-button type="primary" :loading="isLoadingNotaries" @click="handleFetchNotaries">查询</el-button>
        </el-form-item>
        <el-form-item>
          <el-button :disabled="!notaryList.length" @click="notaryList = []">清空</el-button>
        </el-form-item>
      </el-form>

      <el-table
        :data="notaryList"
        size="mini"
        border
        style="width: 100%; margin-top: 10px;"
      >
        <el-table-column prop="id" label="id" width="90" />
        <el-table-column prop="ip" label="ip" min-width="200" show-overflow-tooltip />
        <el-table-column prop="trustValue" label="信誉值" width="120" />
      </el-table>
    </el-card>
  </div>
</template>

<script>
import {
  reqRegisterGetTrustValuebyId,
  reqRegisterRegisterlist,
  reqRegisterResult,
  reqRegisterStoreCipherData
} from '@/api/fisco'
import { reqGetTransactionReceipt as reqWeb3TransactionReceipt } from '@/api/webase-monitor'
import { reqEncodeCipherData } from '@/api/crypto'
import { reqListLocalKeyStores } from '@/api/webase-keys'
import registerHero from '@/assets/feature-images/register-lock.svg'

const REGISTER_ABI = [
  { inputs: [{ internalType: 'address', name: 'oracleAddress', type: 'address' }], stateMutability: 'nonpayable', type: 'constructor' },
  {
    inputs: [
      { internalType: 'string', name: '_ip', type: 'string' },
      { internalType: 'string', name: '_ciphertext', type: 'string' },
      { internalType: 'string', name: '_iv', type: 'string' },
      { internalType: 'string', name: '_tag', type: 'string' },
      { internalType: 'string', name: '_aad', type: 'string' },
      { internalType: 'string', name: '_pub', type: 'string' }
    ],
    name: 'storeCipherData',
    outputs: [{ internalType: 'bytes32', name: '', type: 'bytes32' }],
    stateMutability: 'nonpayable',
    type: 'function'
  },
  // public string result;  => function result() view returns (string)
  { inputs: [], name: 'result', outputs: [{ internalType: 'string', name: '', type: 'string' }], stateMutability: 'view', type: 'function' },
  {
    inputs: [{ internalType: 'uint256', name: '_id', type: 'uint256' }],
    name: 'getTrustValuebyId',
    outputs: [{ internalType: 'uint256', name: '', type: 'uint256' }],
    stateMutability: 'view',
    type: 'function'
  },
  // public registerlist getter
  {
    inputs: [{ internalType: 'uint256', name: '', type: 'uint256' }],
    name: 'registerlist',
    outputs: [
      { internalType: 'uint256', name: 'Notary_id', type: 'uint256' },
      { internalType: 'string', name: 'Notary_ip', type: 'string' },
      { internalType: 'string', name: 'ciphertext', type: 'string' },
      { internalType: 'string', name: 'iv', type: 'string' },
      { internalType: 'string', name: 'tag', type: 'string' },
      { internalType: 'string', name: 'aad', type: 'string' },
      { internalType: 'string', name: 'pub', type: 'string' }
    ],
    stateMutability: 'view',
    type: 'function'
  }
]

const DEFAULT_REGISTER_CONTRACT_ADDRESS = '0x3a015b6e989ce55dcdd0bf6c58d3875761b98dea'

export default {
  name: 'RegisterContract',
  data() {
    return {
      heroImg: registerHero,
      groupId: 1,
      userAddress: '',
      selectedWeBaseAddress: '',
      isLoadingWeBaseAccounts: false,
      weBaseAccounts: [],
      contractAddress: DEFAULT_REGISTER_CONTRACT_ADDRESS,
      contractAbi: REGISTER_ABI,

      form: {
        ip: '',
        ciphertext: '',
        iv: '',
        tag: '',
        aad: '',
        pub: ''
      },

      encodeForm: {
        asset1: '',
        asset2: '',
        asset3: ''
      },
      isEncoding: false,
      encodeResult: null,

      isTransacting: false,
      txReceipt: null,

      isWaitingCallback: false,
      callbackResult: '',
      callbackPollTimer: null,
      callbackStartAt: null,
      successDialogVisible: false,

      registeredId: null,

      trustId: 0,
      isQueryingTrust: false,
      trustValue: null,

      notaryIdFrom: 0,
      notaryIdTo: 10,
      isLoadingNotaries: false,
      notaryList: []
    }
  },
  computed: {
    registeredIdDisplay() {
      return this.registeredId == null ? '-' : String(this.registeredId)
    },
    trustValueDisplay() {
      if (this.trustValue === null || this.trustValue === undefined) return '暂无数据'
      return String(this.trustValue)
    }
  },
  created() {
    const addr = this.$store.getters.userAddress
    if (addr && !this.userAddress) this.userAddress = addr
    this.fetchWeBaseAccounts()
  },
  beforeDestroy() {
    this.stopCallbackPolling()
  },
  methods: {
    normalizeAddress(addr) {
      return String(addr || '').trim()
    },
    formatWeBaseAccountLabel(item) {
      const address = this.normalizeAddress(item.address)
      const userName = item.userName ? String(item.userName) : ''
      const type = item.type != null ? String(item.type) : ''
      const parts = []
      if (userName) parts.push(userName)
      if (type) parts.push(`type=${type}`)
      return parts.length ? `${address} (${parts.join(', ')})` : address
    },
    applySelectedWeBaseAddress(val) {
      const v = this.normalizeAddress(val)
      if (v) {
        this.userAddress = v
        this.$store.commit('user/SET_USER_ADDRESS', v)
      }
    },
    async fetchWeBaseAccounts() {
      this.isLoadingWeBaseAccounts = true
      try {
        const resp = await reqListLocalKeyStores({ silent: true })
        const data = resp && (resp.data != null ? resp.data : resp)
        const list = Array.isArray(data) ? data : (data && Array.isArray(data.data) ? data.data : [])

        this.weBaseAccounts = (list || [])
          .map(i => ({
            address: this.normalizeAddress(i && i.address),
            userName: i && i.userName,
            type: i && i.type
          }))
          .filter(i => i.address)

        const current = this.normalizeAddress(this.userAddress).toLowerCase()
        const matched = this.weBaseAccounts.find(i => this.normalizeAddress(i.address).toLowerCase() === current)
        if (matched) {
          this.selectedWeBaseAddress = matched.address
        } else if (this.weBaseAccounts.length) {
          this.selectedWeBaseAddress = this.weBaseAccounts[0].address
          this.applySelectedWeBaseAddress(this.selectedWeBaseAddress)
          if (current) this.$message.warning('当前 User 地址在 WeBASE-Front 本地私钥列表中不存在，已切换为可签名账户')
        }
      } catch (e) {
        // eslint-disable-next-line no-console
        console.error(e)
        this.$message.error('获取 WeBASE-Front 账户列表失败，请检查 /webase-api 代理与 WeBASE-Front 服务')
      } finally {
        this.isLoadingWeBaseAccounts = false
      }
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
    parseRegisterlistResp(resp, fallbackId) {
      if (!resp) return { id: fallbackId, ip: '' }
      if (Array.isArray(resp) && resp.length >= 2) {
        const id = this.normalizeNumber(resp[0])
        const ip = resp[1] != null ? String(resp[1]) : ''
        return { id: id != null ? id : fallbackId, ip }
      }
      if (resp && typeof resp === 'object') {
        const id = this.normalizeNumber(resp.Notary_id != null ? resp.Notary_id : resp.notaryId)
        const ip = resp.Notary_ip != null ? String(resp.Notary_ip) : (resp.notaryIp != null ? String(resp.notaryIp) : '')
        return { id: id != null ? id : fallbackId, ip }
      }
      return { id: fallbackId, ip: '' }
    },
    async handleEncode() {
      const asset1 = String(this.encodeForm.asset1 || '').trim()
      const asset2 = String(this.encodeForm.asset2 || '').trim()
      const asset3 = String(this.encodeForm.asset3 || '').trim()

      if (!asset1 || !asset2 || !asset3) {
        this.$message.warning('请填写 Asset1/Asset2/Asset3')
        return
      }
      if (!this.$store.getters.token) {
        this.$message.warning('请先登录后再加密')
        return
      }

      this.isEncoding = true
      this.encodeResult = null
      try {
        const resp = await reqEncodeCipherData({
          Asset1: asset1,
          Asset2: asset2,
          Asset3: asset3
        })
        this.encodeResult = resp && resp.data ? resp.data : null
        if (!this.encodeResult) {
          this.$message.error('加密失败：无返回数据')
          return
        }
        this.applyEncodeResult()
        this.$message.success('加密成功，已填入 storeCipherData 表单')
      } catch (e) {
        // eslint-disable-next-line no-console
        console.error(e)
        const status = e && e.response && e.response.status
        if (status === 404) {
          this.$message.error('加密接口不存在（404）。请确认已重启后端 `server`，并包含 /api/crypto/encode 路由')
        } else if (status === 401) {
          this.$message.error('未登录或登录已过期，请重新登录后再试')
        } else {
          this.$message.error('加密失败，请检查后端服务与 Encode.py 依赖')
        }
      } finally {
        this.isEncoding = false
      }
    },
    applyEncodeResult() {
      if (!this.encodeResult) return
      this.form.ciphertext = this.encodeResult.ciphertext || ''
      this.form.iv = this.encodeResult.iv || ''
      this.form.tag = this.encodeResult.tag || ''
      this.form.aad = this.encodeResult.aad || ''
      this.form.pub = this.encodeResult.pub || ''
    },
    clearEncodeResult() {
      this.encodeResult = null
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
    normalizeCallbackResult(val) {
      return String(val || '').trim().toLowerCase()
    },
    startCallbackPolling() {
      this.stopCallbackPolling()
      this.isWaitingCallback = true
      this.callbackResult = ''
      this.callbackStartAt = Date.now()
      this.pollCallbackOnce()
      this.callbackPollTimer = setInterval(() => {
        this.pollCallbackOnce()
      }, 2500)
    },
    stopCallbackPolling() {
      if (this.callbackPollTimer) {
        clearInterval(this.callbackPollTimer)
        this.callbackPollTimer = null
      }
      this.isWaitingCallback = false
    },
    async pollCallbackOnce() {
      if (!this.ensureBaseParams()) return
      if (!this.callbackStartAt) this.callbackStartAt = Date.now()
      if (Date.now() - this.callbackStartAt > 3 * 60 * 1000) {
        this.stopCallbackPolling()
        this.$message.warning('等待回调超时（> 3 分钟），请稍后手动查看链上结果')
        return
      }
      try {
        const response = await reqRegisterResult({
          userAddress: this.userAddress,
          contractAddress: this.contractAddress,
          abi: this.contractAbi
        })
        const result = Array.isArray(response) && response.length ? response[0] : response
        const normalized = this.normalizeCallbackResult(result)
        if (result) this.callbackResult = String(result)
        if (normalized === 'ok') {
          this.stopCallbackPolling()
          await this.ensureRegisteredId()
          this.successDialogVisible = true
        }
      } catch (e) {
        // ignore transient errors, keep polling
      }
    },
    async handleStoreCipherData() {
      if (!this.ensureBaseParams()) return
      if (!this.form.ip || !this.form.ciphertext) {
        this.$message.warning('请至少填写 IP 与 ciphertext')
        return
      }

      this.isTransacting = true
      this.txReceipt = null
      this.registeredId = null
      this.stopCallbackPolling()
      try {
        const response = await reqRegisterStoreCipherData({
          userAddress: this.userAddress,
          contractAddress: this.contractAddress,
          ip: this.form.ip,
          ciphertext: this.form.ciphertext,
          iv: this.form.iv || '',
          tag: this.form.tag || '',
          aad: this.form.aad || '',
          pub: this.form.pub || '',
          abi: this.contractAbi
        })

        this.txReceipt = response
        this.registeredId = this.extractNotaryIdFromReceipt(response, this.contractAddress)
        if (this.registeredId == null && response && response.transactionHash) {
          await this.ensureRegisteredId()
        }
        if (response && response.status === '0x0') {
          this.$message.success('交易上链成功！')
          this.startCallbackPolling()
        } else {
          this.$message.warning('交易已返回，但状态可能异常，请检查回执/控制台')
          // eslint-disable-next-line no-console
          console.warn('Register.storeCipherData 响应:', response)
        }
      } catch (e) {
        // eslint-disable-next-line no-console
        console.error(e)
        this.$message.error('调用失败，请检查 WeBASE-Front 配置与合约信息')
      } finally {
        this.isTransacting = false
      }
    },
    async ensureRegisteredId() {
      if (this.registeredId != null) return
      if (!this.contractAddress) return

      // 1) 优先从当前回执里解析
      if (this.txReceipt) {
        const id = this.extractNotaryIdFromReceipt(this.txReceipt, this.contractAddress)
        if (id != null) {
          this.registeredId = id
          return
        }
      }

      // 2) 某些 WeBASE-Front trans/handle 回执不带 logs：再通过 web3/transactionReceipt 拉一次
      const txHash = this.txReceipt && this.txReceipt.transactionHash
      if (!txHash) return
      try {
        const receiptResp = await reqWeb3TransactionReceipt(this.groupId, txHash, { silent: true })
        const id = this.extractNotaryIdFromReceipt(receiptResp, this.contractAddress)
        if (id != null) this.registeredId = id
      } catch (e) {
        // ignore
      }
    },
    async copyText(text, successMsg) {
      const value = String(text || '').trim()
      if (!value) {
        this.$message.warning('内容为空，无法复制')
        return
      }
      try {
        if (navigator && navigator.clipboard && navigator.clipboard.writeText) {
          await navigator.clipboard.writeText(value)
        } else {
          const el = document.createElement('textarea')
          el.value = value
          el.setAttribute('readonly', '')
          el.style.position = 'absolute'
          el.style.left = '-9999px'
          document.body.appendChild(el)
          el.select()
          document.execCommand('copy')
          document.body.removeChild(el)
        }
        this.$message.success(successMsg || '已复制')
      } catch (e) {
        this.$message.error('复制失败，请手动选择复制')
      }
    },
    copyAllSuccessInfo() {
      const txHash = (this.txReceipt && this.txReceipt.transactionHash) || ''
      const lines = [
        `transactionHash: ${txHash || '-'}`,
        `contractAddress: ${this.contractAddress || '-'}`,
        `user: ${this.userAddress || '-'}`,
        `Notary_id: ${this.registeredIdDisplay}`
      ]
      this.copyText(lines.join('\n'), '已复制全部信息')
    },
    unwrapReceiptLike(obj) {
      if (!obj || typeof obj !== 'object') return obj
      // 兼容不同返回结构：{ result: {...} } / { data: {...} } / { receipt: {...} }
      if (obj.result && typeof obj.result === 'object') return obj.result
      if (obj.data && typeof obj.data === 'object') return obj.data
      if (obj.receipt && typeof obj.receipt === 'object') return obj.receipt
      return obj
    },
    normalizeTopics(topics) {
      if (!topics) return []
      if (Array.isArray(topics)) {
        return topics
          .map(t => {
            if (typeof t === 'string') return t
            if (t && typeof t === 'object' && typeof t.value === 'string') return t.value
            return ''
          })
          .filter(Boolean)
      }
      if (typeof topics === 'string') {
        // 有些返回可能是 "0x...,0x..." 这种格式
        return topics.split(',').map(s => s.trim()).filter(Boolean)
      }
      return []
    },
    extractNotaryIdFromReceipt(receipt, contractAddress) {
      if (!receipt || !contractAddress) return null
      const r = this.unwrapReceiptLike(receipt)
      const logs = (r && (r.logs || r.logEntries || r.log || r.receiptLogs)) || []
      if (!Array.isArray(logs) || !logs.length) return null

      const target = String(contractAddress).toLowerCase()
      const candidates = logs.filter(l => l && String(l.address || '').toLowerCase() === target)
      const picked = (candidates.length ? candidates : logs).find(l => {
        const topics = this.normalizeTopics(l && (l.topics || l.topic))
        return topics.length >= 2
      })
      if (!picked) return null

      const topics = this.normalizeTopics(picked.topics || picked.topic)
      if (topics.length < 2) return null

      const idHex = topics[1]
      if (!idHex || typeof idHex !== 'string' || !/^0x[0-9a-fA-F]+$/.test(idHex)) return null

      try {
        // _id 为自增小整数，这里用 parseInt 足够（避免 BigInt 被旧 ESLint 规则报 no-undef）
        return String(parseInt(idHex, 16))
      } catch (e) {
        return null
      }
    },
    async handleGetTrustValuebyId() {
      if (!this.ensureBaseParams()) return
      this.isQueryingTrust = true
      try {
        const response = await reqRegisterGetTrustValuebyId({
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
        this.isQueryingTrust = false
      }
    },
    async handleFetchNotaries() {
      if (!this.ensureBaseParams()) return
      const from = Math.min(this.notaryIdFrom, this.notaryIdTo)
      const to = Math.max(this.notaryIdFrom, this.notaryIdTo)
      const total = to - from + 1
      if (total > 50) {
        this.$message.warning('查询范围过大（>50），请缩小 id 范围')
        return
      }

      this.isLoadingNotaries = true
      try {
        const rows = []
        for (let id = from; id <= to; id++) {
          try {
            const infoResp = await reqRegisterRegisterlist({
              userAddress: this.userAddress,
              contractAddress: this.contractAddress,
              id: String(id),
              abi: this.contractAbi
            })
            const infoArr = Array.isArray(infoResp) && infoResp.length ? infoResp : infoResp
            const info = this.parseRegisterlistResp(infoArr, id)

            const trustResp = await reqRegisterGetTrustValuebyId({
              userAddress: this.userAddress,
              contractAddress: this.contractAddress,
              id: String(id),
              abi: this.contractAbi
            })
            const trustVal = Array.isArray(trustResp) && trustResp.length ? trustResp[0] : trustResp

            rows.push({
              id: info.id,
              ip: info.ip || '-',
              trustValue: trustVal != null ? String(trustVal) : '-'
            })
          } catch (e) {
            // 越界时通常直接报错：后续 id 也不再存在，直接停止
            break
          }
        }

        this.notaryList = rows
        this.$message.success(`获取完成：${rows.length} 条`)
      } catch (e) {
        // eslint-disable-next-line no-console
        console.error(e)
        this.$message.error('获取失败')
      } finally {
        this.isLoadingNotaries = false
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
  background: linear-gradient(135deg, #1f2d3d 0%, #2b2f55 45%, #4f46e5 100%);
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
.copy-row {
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: 8px;
  width: 100%;
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
