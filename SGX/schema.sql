CREATE TABLE notary_data (
    id          INT AUTO_INCREMENT PRIMARY KEY,
    ciphertext  VARCHAR(512)  NOT NULL,   -- 变长密文 hex
    iv          CHAR(24)      NOT NULL,   -- 12字节 固定
    tag         CHAR(32)      NOT NULL,   -- 16字节 固定
    aad         CHAR(36)      NOT NULL,   -- 18字节 固定
    pub_key     CHAR(128)     NOT NULL,   -- 64字节 固定
    trust       INT           NOT NULL DEFAULT 0,
    state       INT           NOT NULL DEFAULT 0,
    created_at  DATETIME      DEFAULT CURRENT_TIMESTAMP
);
