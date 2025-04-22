# 🔐 Asymmetric Key Cryptography (AKC) Summary

## 🧠 What is Asymmetric Key Cryptography?
- Cryptography that uses **two different keys**:
  - A **Public Key** (shared with everyone)
  - A **Private Key** (kept secret)
- Also known as **Public-Key Cryptography**
- Used for:
  - **Secure communication**
  - **Digital signatures**
  - **Authentication**

---

## 🔑 Key Components

### Public Key
- Shared **openly**
- Used to **encrypt data** or **verify a signature**
- Cannot be used to decrypt what it encrypts

### Private Key
- Kept **secret**
- Used to **decrypt data** encrypted with the public key, or **sign** a message
- Mathematically linked to the public key

---

## 🔁 How It Works

### 🔏 Encryption & 🔓 Decryption
1. **Sender** encrypts message using recipient’s **public key**
2. **Recipient** decrypts using their **private key**

➡️ Ensures **confidentiality**

---

### ✍️ Digital Signature
1. **Sender** creates a hash of the message (e.g., using SHA-256)
2. **Sender signs** the hash using their **private key**
3. **Receiver** verifies the signature using sender’s **public key**

➡️ Ensures **authenticity and integrity**

---

## ⚙️ Common Algorithms
- **RSA(Rivest-Shamir-Adleman)** – Based on prime factorization
- **ECC (Elliptic Curve Cryptography)** – Based on curve math
- **Diffie-Hellman** – Secure key exchange
- **ElGamal**, **DSA**, etc.

---

## 🔐 Key Generation Overview

### RSA Key Generation
1. Choose two large primes `p` and `q`
2. Compute `n = p × q`; This will be half of the public key.
3. Compute `φ(n) = (p−1)(q−1)`
4. Select `e` such that `1 < e < φ(n)` and `gcd(e, φ(n)) = 1`. 
    In practice, `e`, the public exponant, is often chosen to be (2^16+1)=65537, 
    though it can be as small as 33 in some cases. `e` will be the other half of the public key.
5. Calculate  modular inverse `d` (instead of power) such that `d × e ≡ 1 mod φ(n)`. In other words, d is the private key.

- Public Key: `(n, e)`
- Private Key: `(n, d)`
#### Additional notes:
1. The modulus `mod` gives the remainder after division. 
2. Modulo arithmetic is bounded, which keeps values from growing too large.
3. It makes computations cyclic and non-reversible unless you have specific keys.
5. Why Not Use Normal pow()? 
   Because message^e is a massive number that exceeds normal integer sizes and takes longer to compute.

- Encryption: ciphertext = (message ^ e) mod n
- Decryption: message = (ciphertext ^ d) mod n

### ECC Key Generation
1. Choose elliptic curve and base point `G`
2. Select random private key `d`
3. Compute public key: `Q = d × G`

- Public Key: `Q`
- Private Key: `d`

---

## 🛡️ Security Assumptions
- RSA: Hard to factor large numbers
- ECC: Hard to solve elliptic curve discrete logarithm problem
- Secure as long as **keys are long enough** and **randomness is strong**

---

## ✅ Use Cases
- HTTPS / SSL (web security)
- Email encryption (PGP)
- Cryptocurrencies (wallets and signatures)
- Secure file transfers (SSH)
- Digital certificates and signatures

---

## 📌 Summary Table

| Task                  | Key Used      | Purpose                                     |
|-----------------------|---------------|---------------------------------------------|
| Encrypt Data          | Public Key    | Only private key holder can decrypt         |
| Decrypt Data          | Private Key   | To read what was encrypted with public key  |
| Sign a Message        | Private Key   | Prove it came from the sender               |
| Verify a Signature    | Public Key    | Confirm signature authenticity              |
