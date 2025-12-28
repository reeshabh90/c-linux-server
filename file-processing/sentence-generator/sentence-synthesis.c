/**
 * sentence-synthesis V1.0 - Random Synthesis
 * @file: sentence-synthesis.c
 * @author: Reeshabh Choudhary
 *
 * This program generates random natural-sounding sentences
 * using template-based synthesis with proper grammar.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <ctype.h>

// --------------------- DATA SETS ---------------------

const char *names[] = {"Reeshabh", "Richa", "Maithili", "Jaanki", "Hanuman"};
const int names_count = sizeof(names) / sizeof(names[0]);

const char *verbs[] = {"eat", "walk", "write", "talk", "play", "jump", "exercise",
                       "read", "sleep", "code"};
const int verbs_count = sizeof(verbs) / sizeof(verbs[0]);

const char *adverbs[] = {"slowly", "calmly", "mindfully", "fast", "anxiously",
                         "wonderfully", "carefully"};
const int adverbs_count = sizeof(adverbs) / sizeof(adverbs[0]);

const char *pronouns[] = {"He", "She", "They", "People", "We", "I", "You"};
const int pronouns_count = sizeof(pronouns) / sizeof(pronouns[0]);

const char *adjectives[] = {
    "happy", "calm", "energetic", "thoughtful", "careful",
    "quick", "graceful", "noisy", "quiet", "hungry"};
const int adjectives_count = sizeof(adjectives) / sizeof(adjectives[0]);

// --------------------- UTILITIES ---------------------

long long current_milliseconds()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)tv.tv_sec * 1000LL + tv.tv_usec / 1000LL;
}

void conjugate_verb_third_person(const char *verb, char *out, size_t out_size)
{
    strncpy(out, verb, out_size - 3);
    out[out_size - 3] = '\0';

    size_t len = strlen(out);
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

int is_third_person_singular(const char *pronoun)
{
    return (strcmp(pronoun, "He") == 0 || strcmp(pronoun, "She") == 0);
}

// --------------------- TEMPLATE FUNCTIONS ---------------------

void apply_template_1(const char *pronoun, const char *verb, const char *adverb)
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

void apply_template_2(const char *pronoun, const char *verb, const char *adverb)
{
    if (is_third_person_singular(pronoun))
    {
        char verb_s[30];
        conjugate_verb_third_person(verb, verb_s, sizeof(verb_s));
        printf("%s %s %s.\n", pronoun, verb_s, adverb);
    }
    // Only prints for He/She — as per your original intent
}

void apply_template_3(const char *name, const char *verb, const char *adverb)
{
    char verb_s[30];
    conjugate_verb_third_person(verb, verb_s, sizeof(verb_s));
    printf("%s %s %s.\n", name, verb_s, adverb);
}

void apply_template_4(const char *name, const char *adjective, const char *verb, const char *adverb)
{
    char verb_s[30];
    conjugate_verb_third_person(verb, verb_s, sizeof(verb_s));
    printf("%s is %s and %s %s.\n", name, adjective, verb_s, adverb);
}

// --------------------- RANDOM SYNTHESIS ---------------------

void generate_random_sentence()
{
    //long long ms = current_milliseconds();

    // Step 1: Randomly choose a template
    //int template_choice = (ms % 4) + 1;
    int template_choice = (rand() % 4) + 1;
    // ms = current_milliseconds(); // Refresh time

    // Step 2: Randomly select components
    // const char *selected_name = names[ms % names_count];
    // printf("Debug: Selected name: %s\n", selected_name);
    // ms = current_milliseconds();
    // const char *selected_verb = verbs[ms % verbs_count];
    // printf("Debug: Selected verb: %s\n", selected_verb);
    // ms = current_milliseconds();
    // const char *selected_adverb = adverbs[ms % adverbs_count];
    // printf("Debug: Selected adverb: %s\n", selected_adverb);
    // ms = current_milliseconds();
    // const char *selected_adjective = adjectives[ms % adjectives_count];
    // printf("Debug: Selected adjective: %s\n", selected_adjective);
    // ms = current_milliseconds();
    // const char *selected_pronoun = pronouns[ms % pronouns_count];

    const char *selected_name      = names[rand() % names_count];
    const char *selected_verb      = verbs[rand() % verbs_count];
    const char *selected_adverb    = adverbs[rand() % adverbs_count];
    const char *selected_adjective = adjectives[rand() % adjectives_count];
    const char *selected_pronoun   = pronouns[rand() % pronouns_count];


    // Step 3: Apply the chosen template
    switch (template_choice)
    {
    case 1:
        apply_template_1(selected_pronoun, selected_verb, selected_adverb);
        break;
    case 2:
        apply_template_2(selected_pronoun, selected_verb, selected_adverb);
        break;
    case 3:
        apply_template_3(selected_name, selected_verb, selected_adverb);
        break;
    case 4:
        apply_template_4(selected_name, selected_adjective, selected_verb, selected_adverb);
        break;
    }
}

// --------------------- MAIN ---------------------

int main(int argc, char *argv[])
{
    // Default: generate 10 sentences
    int count = 10; 

    if (argc > 1)
    {
        count = atoi(argv[1]);
        if (count <= 0)
            count = 10;
    }

    printf("Generating %d random sentences:\n\n", count);

    for (int i = 0; i < count; i++)
    {
        generate_random_sentence();
    }
    return 0;
}