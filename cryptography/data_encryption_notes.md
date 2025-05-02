
# 🔐 Data Encryption Systems

## DES (Data Encryption Standard)

### Key Characteristics:
- **Block Size**: 64 bits
- **Key Size**: 64 bits  
  - 8 bits used for parity check  
  - **Effective key length**: 56 bits

### Design:
- Based on **Feistel Cipher**
- Uses **16 rounds** of encryption
- Each round includes:
  - Substitution
  - Permutation
  - Left shifting (rotation)
  - Mixing with round key

### Steps in DES:

#### 1. **Key Generation:**
- Initial 64-bit key ➝ 8 bits used for parity (bits: 8, 16, 23, etc.)
- Remaining 56-bit key used for encryption and decryption

#### 2. **Splitting:**
- 56-bit key is divided into two 28-bit blocks
- Each 28-bit block is left-shifted in every round

#### 3. **Combination:**
- Two blocks are combined and processed through compression permutation to form round keys

### 🔓 Why DES is Not Safe:
- **Short key length (56 bits)** is vulnerable to **brute-force attacks**. 
- In 1998, the EFF built a machine that could crack DES in **less than a day**.
- Modern computing power can easily exhaust all 2⁵⁶ possible keys in a short time.
- DES has been officially **deprecated** and replaced by AES.

---

## AES (Advanced Encryption Standard)

### Key Characteristics:
- **Block size**: 128 bits (fixed)
- **Key lengths**: 
  - 128 bits → 10 rounds  
  - 192 bits → 12 rounds  
  - 256 bits → 14 rounds  

### Structure:
- **Substitution-Permutation Network**
- Each block treated as a 4×4 byte matrix (State Array)

### AES Round Steps (1–9 for AES-128):
1. **Substitution Bytes (S-Box)**
2. **Shift Rows**
3. **Mix Columns** (not in the final round)
4. **Add Round Key**

> Each round key is derived from the original key using the **Key Schedule Algorithm**
> Original key is agreed upon in advance by both sender and receiver.
> Original Key is shared secretly (eg. using RSA or secure storage).
> Original Key can be pre-shared, generated from a password using a Key Derivation Function (like PBKDF2, scrypt), or cryptographically secure random number generator and securely exchanged (e.g., via RSA). 

### Key Generation + Secure Sharing 
1. 128-bit key is generated from cryptographically secure random number generator.
2. Encrypt AES Key using Receiver’s RSA Public Key
3. Send Encrypted AES Key + Encrypted Data
4. Receiver Decrypts AES Key with RSA Private Key
5. Receiver Uses AES Key to Decrypt the Data

### Key Schedule Process:
1. Byte Substitution (S-box)
2. Word Rotation
3. XOR with Round Constant (Rcon)
4. Combine with previous word (key expansion)

### 🔐 What Makes AES Secure:
- **Large key sizes (128/192/256 bits)** make brute-force attacks infeasible.
- Uses strong **non-linear transformations** and **multiple rounds** of encryption.
- AES has been **extensively analyzed** and is resistant to all known practical cryptographic attacks.
- Widely used across the world for government, financial, and secure communications.

### Decryption:
- 🔐 Inputs
  - Ciphertext (C)
  - Expanded round keys (K0 to K10) — same as encryption
  - AES standard decryption logic (Inv operations)
  - Same round keys are used **in reverse order**
    - Each step is the inverse of the corresponding encryption step:
      1. **Initial Round** : `AddRoundKey(C, K10)`
      2. **Rounds 1 to 9** (in reverse order)
          For each round key `Ki` from `K9` to `K1`:
          - `InvShiftRows()`
          - `InvSubBytes()`
          - `AddRoundKey(Ki)`
          - `InvMixColumns()`
      3. **Final Round**
        - `InvShiftRows()`
        - `InvSubBytes()`
        - `AddRoundKey(K0)`



---

## Notes:
- `AddRoundKey()` is symmetric (XOR operation) — same in encryption and decryption.
- Inverse functions (`InvSubBytes`, `InvShiftRows`, `InvMixColumns`) are mathematically defined to undo their corresponding encryption steps.
- Receiver only needs the original symmetric key to decrypt, as round keys can be regenerated using the same key schedule.
- The key schedule is not reversed — round keys are generated once, then applied in reverse order during decryption.
- AES does **not** use the original key directly in each round.
- All AES processing is done at the **byte level** within the **state matrix**.
