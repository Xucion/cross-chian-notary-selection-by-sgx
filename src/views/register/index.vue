<template>
  <div class="register-container">
    <el-form
      ref="registerForm"
      :model="registerForm"
      :rules="registerRules"
      class="register-form"
      auto-complete="on"
      label-position="left"
      @submit.native.prevent
    >
      <div class="title-container">
        <img class="register-illustration" :src="heroImg" alt="register">
        <h3 class="title">用户注册</h3>
        <div class="subtitle">注册账号后，将账号密码与链上 User 地址进行映射（本地存储示例）</div>
      </div>

      <el-form-item prop="username">
        <span class="svg-container">
          <svg-icon icon-class="user" />
        </span>
        <el-input v-model="registerForm.username" placeholder="Username" name="username" type="text" auto-complete="on" />
      </el-form-item>

      <el-form-item prop="password">
        <span class="svg-container">
          <svg-icon icon-class="password" />
        </span>
        <el-input v-model="registerForm.password" placeholder="Password" name="password" type="password" auto-complete="on" />
      </el-form-item>

      <el-form-item prop="confirmPassword">
        <span class="svg-container">
          <svg-icon icon-class="password" />
        </span>
        <el-input
          v-model="registerForm.confirmPassword"
          placeholder="Confirm Password"
          name="confirmPassword"
          type="password"
          auto-complete="on"
          @keyup.enter.native="handleRegister"
        />
      </el-form-item>

      <el-form-item prop="userAddress">
        <span class="svg-container">
          <i class="el-icon-link" />
        </span>
        <el-input v-model="registerForm.userAddress" placeholder="链上 User 地址（0x...）" name="userAddress" type="text" />
      </el-form-item>

      <el-form-item>
        <span class="svg-container">
          <i class="el-icon-wallet" />
        </span>
        <el-select
          v-model="selectedWeBaseAddress"
          placeholder="从 WeBASE-Front 账户列表选择（推荐）"
          clearable
          filterable
          style="width: 100%;"
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
        <div class="hint-row">
          <el-button type="text" :loading="isLoadingWeBaseAccounts" @click="fetchWeBaseAccounts">刷新 WeBASE-Front 账户</el-button>
          <span class="hint-text">只展示地址，不展示私钥</span>
        </div>
      </el-form-item>

      <el-button :loading="loading" type="primary" style="width:100%;margin-bottom:12px;" @click.native.prevent="handleRegister">
        注册并返回登录
      </el-button>

      <div class="footer">
        <el-button type="text" @click="$router.push('/login')">已有账号？去登录</el-button>
      </div>
    </el-form>
  </div>
</template>

<script>
import registerHero from '@/assets/feature-images/register-lock.svg'
import { register } from '@/api/user'
import { reqListLocalKeyStores } from '@/api/webase-keys'

export default {
  name: 'Register',
  data() {
    const validateUsername = (rule, value, callback) => {
      const v = String(value || '').trim()
      if (!v) return callback(new Error('请输入用户名'))
      if (v.length < 3) return callback(new Error('用户名至少 3 位'))
      return callback()
    }
    const validatePassword = (rule, value, callback) => {
      if (!value || String(value).length < 6) return callback(new Error('密码至少 6 位'))
      return callback()
    }
    const validateConfirm = (rule, value, callback) => {
      if (!value) return callback(new Error('请再次输入密码'))
      if (value !== this.registerForm.password) return callback(new Error('两次密码不一致'))
      return callback()
    }
    const validateUserAddress = (rule, value, callback) => {
      const v = String(value || '').trim()
      if (!v) return callback(new Error('请输入 User 地址'))
      // loose check: allow 0x + 40 hex, but don't block other formats
      if (v.startsWith('0x') && v.length !== 42) return callback(new Error('地址长度看起来不正确（期望 42 位）'))
      return callback()
    }

    return {
      heroImg: registerHero,
      loading: false,
      isLoadingWeBaseAccounts: false,
      weBaseAccounts: [],
      selectedWeBaseAddress: '',
      registerForm: {
        username: '',
        password: '',
        confirmPassword: '',
        userAddress: ''
      },
      registerRules: {
        username: [{ required: true, trigger: 'blur', validator: validateUsername }],
        password: [{ required: true, trigger: 'blur', validator: validatePassword }],
        confirmPassword: [{ required: true, trigger: 'blur', validator: validateConfirm }],
        userAddress: [{ required: true, trigger: 'blur', validator: validateUserAddress }]
      }
    }
  },
  created() {
    this.fetchWeBaseAccounts()
  },
  methods: {
    normalizeAddress(addr) {
      return String(addr || '').trim()
    },
    formatWeBaseAccountLabel(item) {
      if (!item) return ''
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
      if (v) this.registerForm.userAddress = v
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

        // 自动选中第一个地址（如果用户还没填）
        if (!this.registerForm.userAddress && this.weBaseAccounts.length) {
          this.selectedWeBaseAddress = this.weBaseAccounts[0].address
          this.applySelectedWeBaseAddress(this.selectedWeBaseAddress)
        }
      } catch (e) {
        // eslint-disable-next-line no-console
        console.error(e)
        this.$message.error('获取 WeBASE-Front 账户列表失败，请检查 /webase-api 代理与 WeBASE-Front 服务')
      } finally {
        this.isLoadingWeBaseAccounts = false
      }
    },
    handleRegister() {
      this.$refs.registerForm.validate(async valid => {
        if (!valid) return
        this.loading = true
        try {
          await register({
            username: this.registerForm.username,
            password: this.registerForm.password,
            userAddress: this.registerForm.userAddress
          })
          this.$message.success(`注册成功：${this.registerForm.username}`)
          const query = { u: this.registerForm.username }
          this.$router.push({ path: '/login', query })
        } catch (e) {
          this.$message.error(e && e.message ? e.message : '注册失败')
        } finally {
          this.loading = false
        }
      })
    }
  }
}
</script>

<style lang="scss" scoped>
$bg:#2d3a4b;
$dark_gray:#889aa4;
$light_gray:#eee;

.register-container {
  min-height: 100%;
  width: 100%;
  background: radial-gradient(800px 400px at 20% 10%, rgba(79, 70, 229, 0.35), transparent 60%),
    radial-gradient(600px 300px at 80% 20%, rgba(34, 197, 94, 0.25), transparent 55%),
    linear-gradient(135deg, #0f172a 0%, $bg 45%, #111827 100%);
  overflow: hidden;

  .register-form {
    position: relative;
    width: 520px;
    max-width: 100%;
    padding: 54px 35px 36px;
    margin: 10vh auto 0;
    overflow: hidden;
    border-radius: 14px;
    background: rgba(15, 23, 42, 0.55);
    border: 1px solid rgba(255, 255, 255, 0.12);
    box-shadow: 0 20px 60px rgba(0, 0, 0, 0.35);
    backdrop-filter: blur(10px);
  }

  .svg-container {
    padding: 6px 5px 6px 15px;
    color: $dark_gray;
    vertical-align: middle;
    width: 30px;
    display: inline-block;
  }

  .title-container {
    position: relative;

    .register-illustration {
      display: block;
      width: 220px;
      max-width: 70%;
      height: auto;
      margin: 0 auto 10px auto;
      opacity: 0.95;
      filter: drop-shadow(0 14px 28px rgba(0, 0, 0, 0.35));
    }

    .title {
      font-size: 26px;
      color: $light_gray;
      margin: 0px auto 14px auto;
      text-align: center;
      font-weight: bold;
    }

    .subtitle {
      margin: 0 auto 26px auto;
      text-align: center;
      font-size: 13px;
      color: rgba(238, 238, 238, 0.85);
      line-height: 1.6;
    }
  }

  .footer {
    text-align: center;
  }

  .hint-row {
    display: flex;
    align-items: center;
    justify-content: space-between;
    margin-top: 4px;
  }
  .hint-text {
    font-size: 12px;
    color: rgba(238, 238, 238, 0.7);
  }
}
</style>
