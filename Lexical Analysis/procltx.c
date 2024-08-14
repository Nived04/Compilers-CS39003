#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "lex.yy.c"

#define ALPHABET_SIZE 256

// Trie node definition
typedef struct TrieNode {
    struct TrieNode* children[ALPHABET_SIZE];
    int frequency;
    bool isEndOfWord;
} TrieNode;

// Function to create a new trie node
TrieNode* createNode() {
    TrieNode* node = (TrieNode*)malloc(sizeof(TrieNode));
    node->frequency = 0;
    node->isEndOfWord = false;
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        node->children[i] = NULL;
    }
    return node;
}

// Function to insert a word into the trie
void insert(TrieNode* root, const char* word) {
    TrieNode* node = root;
    while (*word) {
        unsigned char index = (unsigned char)*word;
        if (node->children[index] == NULL) {
            node->children[index] = createNode();
        }
        node = node->children[index];
        word++;
    }
    node->frequency++;
    node->isEndOfWord = true;
}

// Function to search for a word in the trie and return its frequency
int search(TrieNode* root, const char* word) {
    TrieNode* node = root;
    while (*word) {
        unsigned char index = (unsigned char)*word;
        if (node->children[index] == NULL) {
            return 0;
        }
        node = node->children[index];
        word++;
    }
    if(node != NULL && node->isEndOfWord)
        return node->frequency;
    return 0;
}

const int max_db_size = 1e5;

void printNames(TrieNode* node, char* wordbase, int depth) {
    if (node == NULL) return;

    if (node->isEndOfWord) {
        wordbase[depth] = '\0';
        printf("\t%s (%d)\n", wordbase, node->frequency);
    }

    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (node->children[i] != NULL) {
            wordbase[depth] = (char)i;
            printNames(node->children[i], wordbase, depth + 1);
        }
    }
}

void populateDB(TrieNode* t) {
    char wordbase[max_db_size];
    printNames(t, wordbase, 0);
}

char* findEnvName(const char* yytxt, int length) {
    int x = 0;
    for(int i=0; i<length; i++) {
        if(yytxt[i] == (char)32) {
            continue;
        }
        else if(yytxt[i] == '{') {
            x = i+1;
            break;
        }
    }
    int env_len = length - x - 1;
    char *env_name = (char *)malloc((env_len+1)*sizeof(char));
    strncpy(env_name, yytext + x, env_len);
    return env_name;
}

void freeTrie(TrieNode* root) {
    if (root == NULL) return;
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        freeTrie(root->children[i]);
    }
    free(root);
}

int main () {
    int next_token;

    TrieNode* root_command = createNode();
    TrieNode* root_env = createNode();

    int display_math_count = 0;
    int inline_math_count = 0;

    TrieNode* root_type[2] = {root_command, root_env};

    while ((next_token = yylex())) {
        switch(next_token) {
            case DISPLAY_MATH:
                display_math_count++;
                break;
            case INLINE_MATH:
                inline_math_count++;
                break;
            case ENV_BEGIN:
            {
                char* env_name = findEnvName(yytext, yyleng);
                insert(root_type[ENV_BEGIN-1], env_name); 
                free(env_name);
                break;
            }
            case COMMAND:
                insert(root_type[COMMAND-1], yytext);
                break;
        }
    }

    printf("Commands used:\n");
    populateDB(root_command);

    printf("Environments used:\n");
    populateDB(root_env);

    printf("%d math equations found\n", inline_math_count/2);
    printf("%d displayed equations found\n", display_math_count/2);
    
    freeTrie(root_type[0]);
    freeTrie(root_type[1]);

    exit(0);
}