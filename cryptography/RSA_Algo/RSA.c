/**
 * 📔RSA.0📔
 * @file: RSA.c
 * @author: Reeshabh Choudhary
 *
 * ℹ️ This program implements RSA algorithm.
 *
 * 1. The idea of RSA is based on the fact that it is difficult to factorize a large integer.
 * 2. Choose two large primes `p` and `q`
 * 3. Compute `n = p × q`; This will be half of the public key.
 * 4. Compute `φ(n) = (p−1)(q−1)`
 * 5. Select `e` such that `1 < e < φ(n)` and `gcd(e, φ(n)) = 1`.
 *    In practice, `e` is often chosen to be (2^16+1)=65537, though it can be as small as 33 in some cases.
 *    `e` will be the other half of the public key.
 * 6. Calculate  modular inverse `d` such that `d × e ≡ 1 mod φ(n)`. In other words, d is the private key.
 * 7. Note: RSA Algorithm takes the value of p and q to be very large which in turn makes the value of n extremely large
 *    and factorizing such a large value is computationally impossible. RSA keys can be typically 1024 or 2048 bits long.
 *
 * ⚠️ Potential Threat: Quantum algorithms, for instance Shor's algorithm, can quickly factor large numbers,
 *   thus making RSA obsolete. Against traditional attacks, with large keys (2048 bit or more) RSA is secure.
 *   However, advancement in computing powers may prove to be a challenge to RSA's reliability in the future.
 *
 * 🫵 To handle this potential risk, post-quantum cryptography, like lattice-based encryption is currently being explored
 *    as a solution to ensure data security in a world where RSA is breakable.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// Defining an alias for long long data type using 'typedef'
typedef long long ll;

/// @brief This function checks whether a given number is prime or not
/// @param n input number
/// @return
int is_prime(ll n)
{
    if (n <= 1)
        return 0;
    if (n <= 3)
        return 1;
    if (n % 2 == 0 || n % 3 == 0)
        return 0;

    for (ll i = 5; i * i <= n; i += 6)
        if (n % i == 0 || n % (i + 2) == 0)
            return 0;
    return 1;
}

/// @brief This function generates a random prime in the given range
/// @param lower range
/// @param upper range
/// @return
ll generate_random_prime(ll lower, ll upper)
{
    ll num;
    do
    {
        num = (rand() % (upper - lower + 1)) + lower;
    } while (!is_prime(num));
    return num;
}

/// @brief This function applies Euler's Totient on an input n.
/// @details Euler’s Totient function Φ(n) for an input n is the count of numbers in {1, 2, 3, …, n-1}
/// that are relatively prime to n, i.e., the numbers whose GCD (Greatest Common Divisor) with n is 1.
/// If n is a prime number: ϕ(n) = n − 1 (because all numbers less than n are coprime to n).
/// @param n a prime number
/// @return (n-1)
ll calculate_euler_totient(ll n)
{
    return (n - 1);
}

/// @brief The GCD is the greatest number that divides a set of numbers without leaving a remainder.
/// @details This function finds the Greatest Common Divisor (GCD) of two numbers a and b.
/// In RSA, we need the public exponent e to be coprime with φ(n) (Euler’s totient function).
/// @param a number
/// @param b number
/// @return
ll calculate_gcd(ll a, ll b)
{
    while (b != 0)
    {
        ll temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

/// @details This function calculates modular multiplicative inverse using the Extended Euclidean Algorithm,
/// which not only computes the GCD but also finds integers x and y such that: a * x + b * y = gcd(a, b).
/// If gcd(e, φ(n)) = 1, then this gives you -> d = x mod φ(n).
/// In simpler terms: `d` is the number you multiply with e to get a remainder of 1 when divided by φ(n).
/// The number `d` becomes the private key.
/// @brief This function returns the private key 'd' such that `d × e ≡ 1 (mod φ)`, using the Extended Euclidean Algorithm.
/// @param e
/// @param phi
/// @return
ll calculate_mod_inverse(ll e, ll phi)
{
    ll coeff_phi = 0, coeff_e = 1;
    ll current_remainder = phi, next_remainder = e;

    while (next_remainder != 0)
    {
        ll quotient = current_remainder / next_remainder;
        ll temp = coeff_e;
        coeff_e = coeff_phi - quotient * coeff_e;
        coeff_phi = temp;

        temp = next_remainder;
        next_remainder = current_remainder - quotient * next_remainder;
        current_remainder = temp;
    }

    if (current_remainder > 1)
        return -1; // Not invertible
    if (coeff_phi < 0)
        coeff_phi += phi;
    return coeff_phi;
}

/// @brief This function calculates Modular exponentiation: (base ^ exponent) % modulus
/// @details Encryption: ciphertext = (message ^ e) mod n
///          Decryption: message = (ciphertext ^ d) mod n
/// @param base
/// @param exp
/// @param mod
/// @return
ll calculate_mod_pow(ll base, ll exp, ll mod)
{
    ll result = 1;
    base = base % mod;

    while (exp > 0)
    {
        if (exp % 2 == 1)
            result = (result * base) % mod;

        base = (base * base) % mod;
        exp /= 2;
    }

    return result;
}

int main(int argc, char const *argv[])
{
    /* code */
    srand(time(NULL)); // Seed RNG

    // Randomly generate two different primes
    ll p = generate_random_prime(100, 500);
    ll q;
    do
    {
        q = generate_random_prime(100, 500);
    } while (q == p);

    printf("Random Prime p = %lld\n", p);
    printf("Random Prime q = %lld\n", q);

    ll n = p * q;
    ll phi = calculate_euler_totient(p) * calculate_euler_totient(q);
    ll e = 65537; // the public exponent

    if (calculate_gcd(e, phi) != 1)
    {
        printf("65537 is not coprime with `phi φ(n)`. Retry\n");
        return 1;
    }

    ll d = calculate_mod_inverse(e, phi);
    if (d == -1)
    {
        printf("Failed to compute modular inverse. Try again.\n");
        return 1;
    }

    printf("\nPublic Key: (n = %lld, e = %lld)\n", n, e);
    printf("Private Key: d = %lld\n", d);

    // Example: Encrypt and Decrypt a message
    ll message;
    printf("\nEnter a number to encrypt (must be < %lld): ", n);
    scanf("%lld", &message);

    if (message >= n)
    {
        printf("Message must be smaller than n.\n");
        return 1;
    }

    ll encrypted = calculate_mod_pow(message, e, n);
    ll decrypted = calculate_mod_pow(encrypted, d, n);

    printf("Encrypted Message: %lld\n", encrypted);
    printf("Decrypted Message: %lld\n", decrypted);

    return 0;
}
