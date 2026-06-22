from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives.ciphers.aead import AESGCM
import os
import random
import mysql.connector

# ===== MySQL 连接配置 =====
db_config = {
    "host": "127.0.0.1",
    "port": 3306,
    "user": "sgx_user",
    "password": "2628",
    "database": "sgx_db"
}

# ===== Enclave 公钥（小端格式转大端）=====
#X = 09FE84177DF0F51E1B3C32CA347F4821F3AA81364D60677E36E9A0117CD51DC8
#Y = 284E9334F773304F5EC968F93094FD4F705CD5B8E46B29A99CD1D33BAD35A178



x_le = bytes.fromhex('09FE84177DF0F51E1B3C32CA347F4821F3AA81364D60677E36E9A0117CD51DC8')
y_le = bytes.fromhex('284E9334F773304F5EC968F93094FD4F705CD5B8E46B29A99CD1D33BAD35A178')

x_be = x_le[::-1]
y_be = y_le[::-1]
x = int.from_bytes(x_be, 'big')
y = int.from_bytes(y_be, 'big')

peer_public_numbers = ec.EllipticCurvePublicNumbers(x, y, ec.SECP256R1())
peer_public_key = peer_public_numbers.public_key(default_backend())

def random_plaintext():
    values = [f"{random.randint(0, 100)}.000" for _ in range(3)]
    return "|".join(values).encode("utf-8")

# ===== 指定生成的公证人数量 =====
NUM_NOTARIES = 3500

# ===== 连接数据库 =====
conn = mysql.connector.connect(**db_config)
cursor = conn.cursor()
# ===== 清空表并重置自增 ID =====
print("清空 notary_data 表并重置 ID...")
cursor.execute("TRUNCATE TABLE notary_data")
conn.commit()
print("清空完成。\n")

print(f"开始生成 {NUM_NOTARIES} 个公证人密钥并加密...\n")

for i in range(NUM_NOTARIES):
    plaintext = random_plaintext()

    # 1. 自动生成用户临时私钥
    private_key = ec.generate_private_key(ec.SECP256R1(), default_backend())
    private_value = private_key.private_numbers().private_value

    # 2. 派生公钥坐标
    public_numbers = private_key.public_key().public_numbers()
    pub_x = public_numbers.x
    pub_y = public_numbers.y

    # pub_hex：X Y 各32字节小端拼接（与原代码格式一致）
    pub_hex = pub_x.to_bytes(32, 'big')[::-1].hex() + pub_y.to_bytes(32, 'big')[::-1].hex()

    # 3. ECDH 派生共享密钥
    shared_key = private_key.exchange(ec.ECDH(), peer_public_key)
    aes_key = shared_key[::-1][:16]

    # 4. AES-GCM 加密
    aesgcm = AESGCM(aes_key)
    nonce = os.urandom(12)
    aad = b'optional-auth-data'

    ciphertext_full = aesgcm.encrypt(nonce, plaintext, aad)
    ciphertext = ciphertext_full[:-16]
    tag = ciphertext_full[-16:]

    # 5. 转十六进制
    ciphertext_hex = ciphertext.hex().upper()
    iv_hex         = nonce.hex().upper()
    tag_hex        = tag.hex().upper()
    aad_hex        = aad.hex().upper()
    trust          = 20
    state          = 0

    # 6. 插入数据库
    sql = """
        INSERT INTO notary_data (ciphertext, iv, tag, aad, pub_key, trust, state)
        VALUES (%s, %s, %s, %s, %s, %s, %s)
    """
    cursor.execute(sql, (ciphertext_hex, iv_hex, tag_hex, aad_hex, pub_hex, trust, state))

    if (i + 1) % 100 == 0 or i == 0 or i + 1 == NUM_NOTARIES:
        print(f"已生成 {i+1}/{NUM_NOTARIES} 条，最新 plaintext={plaintext.decode('utf-8')}")

conn.commit()
print(f"所有 {NUM_NOTARIES} 条数据已成功插入数据库。\n")

# ===== 验证：查询数据库最新插入的记录 =====
print("===== 数据库最新记录 =====")
cursor.execute(
    "SELECT id, LEFT(ciphertext,10) AS cipher_prefix, iv, trust, state, created_at "
    "FROM notary_data ORDER BY id DESC LIMIT %s",
    (NUM_NOTARIES,)
)
rows = cursor.fetchall()
print(f"{'id':<5} {'cipher_prefix':<15} {'iv':<26} {'trust':<8} {'state':<7} created_at")
print("-" * 80)
for row in rows:
    print(f"{row[0]:<5} {row[1]:<15} {row[2]:<26} {row[3]:<8} {row[4]:<7} {row[5]}")

cursor.close()
conn.close()
