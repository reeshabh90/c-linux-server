/**
 * sentence-template V1.0📔
 * @file: sentence-template.c
 * @author: Reeshabh Choudhary
 *
 * ℹ️ This program generates sentences using defined templates.
 * It uses distinct arrays of names, verbs, adverbs, adjectives, and pronouns
 * to create different sentences while ensuring correct verb conjugation
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include "sentence-template.h"

// --------------------- DATA SETS ---------------------

const char *names[] = {"Reeshabh", "Richa", "Maithili", "Jaanki", "Hanuman"};

const char *verbs[] = {"eat", "walk", "write", "talk", "play", "jump", "exercise",
                       "read", "sleep", "code"};

const char *adverbs[] = {"slowly", "calmly", "mindfully", "fast", "anxiously",
                         "wonderfully", "carefully"};

const char *pronouns[] = {"He", "She", "They", "People", "We", "I", "You"};

const char *adjectives[] = {
    "happy", "calm", "energetic", "thoughtful", "careful",
    "quick", "graceful", "noisy", "quiet", "hungry"};

/**
 * @brief Conjugates a verb to its third-person singular form.
 * @param verb Input verb in base form
 * @param out Output buffer to hold the conjugated verb
 * @param out_size Size of the output buffer
 */
void conjugate_verb_third_person(const char *verb, char *out, size_t out_size)
{
    strncpy(out, verb, out_size - 2);
    out[out_size - 2] = '\0'; // safety

    size_t len = strlen(out);
    if (len + 2 >= out_size)
    {
        strcat(out, "s");
        return;
    }

    char last = tolower(verb[len - 1]);
    if (last == 'o' || last == 's' || last == 'x' || last == 'z' ||
        (len >= 2 && tolower(verb[len - 2]) == 'c' && last == 'h') ||
        (len >= 2 && tolower(verb[len - 2]) == 's' && last == 'h'))
    {
        strcat(out, "es");
    }
    else if (last == 'y' && len > 1 && !strchr("aeiou", tolower(verb[len - 2])))
    {
        out[len - 1] = '\0';
        strcat(out, "ies");
    }
    else
    {
        strcat(out, "s");
    }
}

/**
 * @brief Checks if the pronoun is third-person singular (He/She).
 * @param pronoun Input pronoun
 * @return 1 if third-person singular, 0 otherwise
 */
int is_third_person_singular(const char *pronoun)
{
    return (strcmp(pronoun, "He") == 0 || strcmp(pronoun, "She") == 0);
}

// --------------------- TEMPLATE FUNCTIONS ---------------------

/**
 * @brief Template 1: Pronoun + Verb + Adverb
 * @param pronoun Input pronoun
 * @param verb Input verb
 * @param adverb Input adverb
 */
void template_1(const char *pronoun, const char *verb, const char *adverb)
{
    if (is_third_person_singular(pronoun))
    {
        char verb_s[30];
        conjugate_verb_third_person(verb, verb_s, sizeof(verb_s));
        printf("%s %s %s.\n", pronoun, verb_s, adverb);
    }
    else
    {
        printf("%s %s %s.\n", pronoun, verb, adverb);
    }
}

void template_2(const char *pronoun, const char *verb, const char *adverb)
{
    if (is_third_person_singular(pronoun))
    {
        char verb_s[30];
        conjugate_verb_third_person(verb, verb_s, sizeof(verb_s));
        printf("%s %s %s.\n", pronoun, verb_s, adverb);
    }
}

void template_3(const char *name, const char *verb, const char *adverb)
{
    char verb_s[30];
    conjugate_verb_third_person(verb, verb_s, sizeof(verb_s));
    printf("%s %s %s.\n", name, verb_s, adverb);
}

void template_4(const char *name, const char *adjective, const char *verb, const char *adverb)
{
    char verb_s[30];
    conjugate_verb_third_person(verb, verb_s, sizeof(verb_s));
    printf("%s is %s and %s %s.\n", name, adjective, verb_s, adverb);
}

int main(int argc, char const *argv[])
{
    clock_t start, stop;
    // Record the start time
    start = clock();
    size_t names_count = ARRAY_SIZE(names);
    size_t verbs_count = ARRAY_SIZE(verbs);
    size_t adverbs_count = ARRAY_SIZE(adverbs);
    size_t adjectives_count = ARRAY_SIZE(adjectives);
    size_t pronouns_count = ARRAY_SIZE(pronouns);

    printf("Generating sentences using modular templates...\n\n");

    long long total = 0;

    for (size_t n = 0; n < names_count; n++)
    {
        for (size_t v = 0; v < verbs_count; v++)
        {
            for (size_t a = 0; a < adverbs_count; a++)
            {
                for (size_t adj = 0; adj < adjectives_count; adj++)
                {
                    for (size_t p = 0; p < pronouns_count; p++)
                    {

                        const char *name = names[n];
                        const char *verb = verbs[v];
                        const char *adverb = adverbs[a];
                        const char *adjective = adjectives[adj];
                        const char *pronoun = pronouns[p];

                        // Apply templates
                        template_1(pronoun, verb, adverb);
                        total++;

                        template_2(pronoun, verb, adverb);
                        if (is_third_person_singular(pronoun))
                            total++; // only printed for He/She

                        template_3(name, verb, adverb);
                        total++;

                        template_4(name, adjective, verb, adverb);
                        total++;
                    }
                }
            }
        }
    }

    printf("\nTotal sentences generated: %lld\n", total);

    double cpu_time_used = ((double)(clock() - start)) / CLOCKS_PER_SEC;
    printf("Time taken: %f seconds\n", cpu_time_used);
    return EXIT_SUCCESS;
}
