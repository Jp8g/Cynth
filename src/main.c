#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <immintrin.h>

typedef enum TokenType {
    TK_INVALID, //           INVALID

    //LITERALS
    TK_IDENTIFIER, //        x
    TK_INT_LITERAL, //       1
    TK_FLOAT_LITERAL, //     0.0
    TK_CHAR_LITERAL, //      'c'
    TK_STRING_LITERAL, //    "string"

    // KEYWORDS
    TK_KW_IF, //             if
    TK_KW_ELSE, //           else
    TK_KW_MUT, //            mut
    TK_KW_FOR, //            for
    TK_KW_WHILE, //          while
    TK_KW_BREAK, //          break
    TK_KW_CONTINUE, //       continue
    TK_KW_COMPTIME, //       comptime
    TK_KW_EMIT, //           emit
    TK_KW_STRUCT, //         struct
    TK_KW_UNION, //          union
    TK_KW_ENUM, //           enum
    TK_KW_RETURN, //         return

    // OPERATORS
    TK_PLUS, //              +
    TK_INCREMENT, //         ++
    TK_MINUS, //             -
    TK_DECREMENT, //         --
    TK_ASTERISK, //          *
    TK_SLASH, //             /
    TK_BITWISE_AND, //       &
    TK_LOGICAL_AND, //       &&
    TK_BITWISE_OR, //        |
    TK_LOGICAL_OR, //        ||
    TK_BITWISE_NOT, //       ~
    TK_LOGICAL_NOT, //       !
    TK_BITWISE_XOR, //       ^
    TK_MODULO, //            %
    TK_ASSIGN, //            =
    TK_EQUAL, //             ==
    TK_PLUS_EQUAL, //        +=
    TK_MINUS_EQUAL, //       -=
    TK_MULT_EQUAL, //        *=
    TK_DIVIDE_EQUAL, //      /=
    TK_SHIFT_LEFT_EQUAL, //  <<=
    TK_SHIFT_RIGHT_EQUAL, // >>=
    TK_SHIFT_LEFT, //        <<
    TK_SHIFT_RIGHT, //       >>
    TK_LESSER, //            <
    TK_GREATER, //           >
    TK_LESSER_EQUAL, //      <=
    TK_GREATER_EQUAL, //     >=
    TK_BITWISE_AND_EQUAL, // &=
    TK_BITWISE_OR_EQUAL, //  |=
    TK_NOT_EQUAL, //         !=
    TK_BITWISE_XOR_EQUAL, // ^=
    TK_MODULO_EQUAL, //      %=
    TK_PERIOD, //            .
    TK_ARROW, //             ->
    TK_QUESTION_MARK, //     ?
    TK_COLON, //             :

    // PUNCTUATORS
    TK_COMMA, //             ,
    TK_OPEN_PAREN, //        (
    TK_CLOSE_PAREN, //       )
    TK_OPEN_BRACKET, //      [
    TK_CLOSE_BRACKET, //     ]
    TK_OPEN_BRACE, //        {
    TK_CLOSE_BRACE, //       }
    TK_SEMICOLON, //         ;
    
    TK_EOF, //               End of file
    TOKEN_COUNT,
} TokenType;

typedef enum DFAState {
    STATE_NONE,
    STATE_START,
    STATE_WHITESPACE,

    // IDENTIFIERS AND LITERALS
    STATE_IDENTIFIER,
    STATE_INT_LITERAL,
    STATE_FLOAT_LITERAL,
    STATE_CHAR_LITERAL,
    STATE_CHAR_LITERAL_ESCAPE,
    STATE_CHAR_LITERAL_END,
    STATE_STRING_LITERAL,
    STATE_STRING_LITERAL_ESCAPE,
    STATE_STRING_LITERAL_END,

    // OPERATORS
    STATE_PLUS,
    STATE_PLUS_EQUALS,
    STATE_PLUS_PLUS,
    STATE_MINUS,
    STATE_MINUS_EQUALS,
    STATE_MINUS_MINUS,
    STATE_MINUS_GREATER,
    STATE_ASTERISK,
    STATE_ASTERISK_EQUALS,
    STATE_SLASH,
    STATE_SLASH_EQUALS,
    STATE_AND,
    STATE_AND_EQUALS,
    STATE_AND_AND,
    STATE_OR,
    STATE_OR_EQUALS,
    STATE_OR_OR,
    STATE_EXCLAMATION_MARK,
    STATE_EXCLAMATION_MARK_EQUALS,
    STATE_TILDE,
    STATE_HAT,
    STATE_HAT_EQUALS,
    STATE_PERCENT,
    STATE_PERCENT_EQUALS,
    STATE_EQUALS,
    STATE_EQUALS_EQUALS,
    STATE_LESSER,
    STATE_GREATER,
    STATE_LESSER_EQUALS,
    STATE_GREATER_EQUALS,
    STATE_SHIFT_LEFT,
    STATE_SHIFT_RIGHT,
    STATE_SHIFT_LEFT_EQUALS,
    STATE_SHIFT_RIGHT_EQUALS,
    STATE_DOT,
    STATE_QUESTION_MARK,
    STATE_COLON,

    // COMMENTS
    STATE_LINE_COMMENT,
    STATE_BLOCK_COMMENT,
    STATE_BLOCK_COMMENT_ASTERISK,

    // PUNCTUATORS
    STATE_COMMA,
    STATE_OPEN_PARENTHESIS,
    STATE_CLOSE_PARENTHESIS,
    STATE_OPEN_BRACKET,
    STATE_CLOSE_BRACKET,
    STATE_OPEN_BRACE,
    STATE_CLOSE_BRACE,
    STATE_SEMICOLON,
    
    STATE_COUNT,
} DFAState;

typedef struct Token {
    uint64_t line;
    uint64_t column;
    char* literal;
    TokenType type;
    uint32_t literalLength;
} Token;

char* OpenFile(const char* path, uint64_t* size) {
    FILE* file = fopen(path, "rb");
    if (!file) return NULL;

    *size = UINT64_MAX;

    if (fseek(file, 0, SEEK_END) == 0) {
        *size = ftell(file);
        rewind(file);

        if (*size == UINT64_MAX) {
            return NULL;
        }

    } else {
        return NULL;
    }

    char* data = malloc((*size + 1) * sizeof(char));

    fread(data, sizeof(char), *size, file);
    fclose(file);
    
    data[*size] = '\0';

    return data;
}

static inline TokenType GetKeyword(char* input, uint64_t inputLength) {
    TokenType tokenType = TK_IDENTIFIER;

    switch (inputLength) {
        case 2: {
            if (input[0] == 'i' && input[1] == 'f') tokenType = TK_KW_IF;

            break;
        }
        case 3: {
            if (input[0] == 'm' && input[1] == 'u' && input[2] == 't') tokenType = TK_KW_MUT;
            else if (input[0] == 'f' && input[1] == 'o' && input[2] == 'r') tokenType = TK_KW_FOR;

            break;
        }
        case 4: {
            if (input[0] == 'e') {
                if (input[1] == 'l' && input[2] == 's' && input[3] == 'e') tokenType = TK_KW_ELSE;
                else if (input[1] == 'm' && input[2] == 'i' && input[3] == 't') tokenType = TK_KW_EMIT;
                else if (input[1] == 'n' && input[2] == 'u' && input[3] == 'm') tokenType = TK_KW_ENUM;
            }

            break;
        }
        case 5: {
            if (input[0] == 'w' && input[1] == 'h' && input[2] == 'i' && input[3] == 'l' && input[4] == 'e') tokenType = TK_KW_WHILE;
            else if (input[0] == 'b' && input[1] == 'r' && input[2] == 'e' && input[3] == 'a' && input[4] == 'k') tokenType = TK_KW_BREAK;
            else if (input[0] == 'u' && input[1] == 'n' && input[2] == 'i' && input[3] == 'o' && input[4] == 'n') tokenType = TK_KW_UNION;

            break;
        }
        case 6: {
            if (input[0] == 'r' && input[1] == 'e' && input[2] == 't' && input[3] == 'u' && input[4] == 'r' && input[5] == 'n') tokenType = TK_KW_RETURN;
            else if (input[0] == 's' && input[1] == 't' && input[2] == 'r' && input[3] == 'u' && input[4] == 'c' && input[5] == 't') tokenType = TK_KW_STRUCT;

            break;
        }
        case 8: {
            if (input[0] == 'c' && input[1] == 'o' && input[2] == 'm' && input[3] == 'p' && input[4] == 't' && input[5] == 'i' && input[6] == 'm' && input[7] == 'e') tokenType = TK_KW_COMPTIME;
            else if (input[0] == 'c' && input[1] == 'o' && input[2] == 'n' && input[3] == 't' && input[4] == 'i' && input[5] == 'n' && input[6] == 'u' && input[7] == 'e') tokenType = TK_KW_CONTINUE;

            break;
        }
    }

    return tokenType;
}

static const TokenType DFAStateToTokenTypeLookup[STATE_COUNT] = {
    [STATE_NONE]                    = TK_INVALID,
    [STATE_START]                   = TK_INVALID,
    [STATE_WHITESPACE]              = TK_INVALID,
    [STATE_IDENTIFIER]              = TK_IDENTIFIER,
    [STATE_INT_LITERAL]             = TK_INT_LITERAL,
    [STATE_FLOAT_LITERAL]           = TK_FLOAT_LITERAL,
    [STATE_CHAR_LITERAL]            = TK_INVALID,
    [STATE_CHAR_LITERAL_ESCAPE]     = TK_INVALID,
    [STATE_CHAR_LITERAL_END]        = TK_CHAR_LITERAL,
    [STATE_STRING_LITERAL]          = TK_INVALID,
    [STATE_STRING_LITERAL_ESCAPE]   = TK_INVALID,
    [STATE_STRING_LITERAL_END]      = TK_STRING_LITERAL,
    [STATE_PLUS]                    = TK_PLUS,
    [STATE_PLUS_EQUALS]             = TK_PLUS_EQUAL,
    [STATE_PLUS_PLUS]               = TK_INCREMENT,
    [STATE_MINUS]                   = TK_MINUS,
    [STATE_MINUS_EQUALS]            = TK_MINUS_EQUAL,
    [STATE_MINUS_MINUS]             = TK_DECREMENT,
    [STATE_MINUS_GREATER]           = TK_ARROW,
    [STATE_ASTERISK]                = TK_ASTERISK,
    [STATE_ASTERISK_EQUALS]         = TK_MULT_EQUAL,
    [STATE_SLASH]                   = TK_SLASH,
    [STATE_SLASH_EQUALS]            = TK_DIVIDE_EQUAL,
    [STATE_AND]                     = TK_BITWISE_AND,
    [STATE_AND_EQUALS]              = TK_BITWISE_AND_EQUAL,
    [STATE_AND_AND]                 = TK_LOGICAL_AND,
    [STATE_OR]                      = TK_BITWISE_OR,
    [STATE_OR_EQUALS]               = TK_BITWISE_OR_EQUAL,
    [STATE_OR_OR]                   = TK_LOGICAL_OR,
    [STATE_EXCLAMATION_MARK]        = TK_LOGICAL_NOT,
    [STATE_EXCLAMATION_MARK_EQUALS] = TK_NOT_EQUAL,
    [STATE_TILDE]                   = TK_BITWISE_NOT,
    [STATE_HAT]                     = TK_BITWISE_XOR,
    [STATE_HAT_EQUALS]              = TK_BITWISE_XOR_EQUAL,
    [STATE_PERCENT]                 = TK_MODULO,
    [STATE_PERCENT_EQUALS]          = TK_MODULO_EQUAL,
    [STATE_EQUALS]                  = TK_ASSIGN,
    [STATE_EQUALS_EQUALS]           = TK_EQUAL,
    [STATE_LESSER]                  = TK_LESSER,
    [STATE_GREATER]                 = TK_GREATER,
    [STATE_LESSER_EQUALS]           = TK_LESSER_EQUAL,
    [STATE_GREATER_EQUALS]          = TK_GREATER_EQUAL,
    [STATE_SHIFT_LEFT]              = TK_SHIFT_LEFT,
    [STATE_SHIFT_RIGHT]             = TK_SHIFT_RIGHT,
    [STATE_SHIFT_LEFT_EQUALS]       = TK_SHIFT_LEFT_EQUAL,
    [STATE_SHIFT_RIGHT_EQUALS]      = TK_SHIFT_RIGHT_EQUAL,
    [STATE_DOT]                     = TK_PERIOD,
    [STATE_QUESTION_MARK]           = TK_QUESTION_MARK,
    [STATE_COLON]                   = TK_COLON,
    [STATE_LINE_COMMENT]            = TK_INVALID,
    [STATE_BLOCK_COMMENT]           = TK_INVALID,
    [STATE_BLOCK_COMMENT_ASTERISK]  = TK_INVALID,
    [STATE_COMMA]                   = TK_COMMA,
    [STATE_OPEN_PARENTHESIS]        = TK_OPEN_PAREN,
    [STATE_CLOSE_PARENTHESIS]       = TK_CLOSE_PAREN,
    [STATE_OPEN_BRACKET]            = TK_OPEN_BRACKET,
    [STATE_CLOSE_BRACKET]           = TK_CLOSE_BRACKET,
    [STATE_OPEN_BRACE]              = TK_OPEN_BRACE,
    [STATE_CLOSE_BRACE]             = TK_CLOSE_BRACE,
    [STATE_SEMICOLON]               = TK_SEMICOLON,
};

typedef DFAState DFATable[STATE_COUNT][128];

#define PUSH(table, state, _char, state2) ((table)[(state)][(_char)] = (state2))

#define NEUTRAL_STATE_COUNT 2

void GenerateDFATable(DFATable table) {
    memset(table, 0, sizeof(DFATable));
    DFAState neutral_states[NEUTRAL_STATE_COUNT] = {STATE_START, STATE_WHITESPACE};

    for (uint8_t i = 0; i < NEUTRAL_STATE_COUNT; i++) {
        for (uint8_t _char = 0; _char < 128; _char++) {
            PUSH(table, STATE_STRING_LITERAL, _char, STATE_STRING_LITERAL);
            PUSH(table, STATE_LINE_COMMENT, _char, STATE_LINE_COMMENT);
            PUSH(table, STATE_BLOCK_COMMENT, _char, STATE_BLOCK_COMMENT);
            PUSH(table, STATE_STRING_LITERAL_ESCAPE, _char, STATE_STRING_LITERAL);
            PUSH(table, STATE_CHAR_LITERAL_ESCAPE, _char, STATE_CHAR_LITERAL);
            PUSH(table, STATE_BLOCK_COMMENT_ASTERISK, _char, STATE_BLOCK_COMMENT);

            if ((unsigned)((_char | 0x20) - 'a') < 26) {
                PUSH(table, neutral_states[i], _char, STATE_IDENTIFIER);
                PUSH(table, STATE_IDENTIFIER, _char, STATE_IDENTIFIER);
            }
            if (((unsigned)(_char - '0') <= 9)) {
                PUSH(table, neutral_states[i], _char, STATE_INT_LITERAL);
                PUSH(table, STATE_INT_LITERAL, _char, STATE_INT_LITERAL);
                PUSH(table, STATE_FLOAT_LITERAL, _char, STATE_FLOAT_LITERAL);
                PUSH(table, STATE_DOT, _char, STATE_FLOAT_LITERAL);
            }
        }

        PUSH(table, neutral_states[i], '_', STATE_IDENTIFIER);
        PUSH(table, STATE_IDENTIFIER, '_', STATE_IDENTIFIER);
        PUSH(table, STATE_INT_LITERAL, '_', STATE_INT_LITERAL);
        PUSH(table, STATE_FLOAT_LITERAL, '_', STATE_FLOAT_LITERAL);
            
        PUSH(table, STATE_INT_LITERAL, '.', STATE_FLOAT_LITERAL);
        PUSH(table, neutral_states[i], '.', STATE_DOT);
            
        PUSH(table, neutral_states[i], '(', STATE_OPEN_PARENTHESIS);
        PUSH(table, neutral_states[i], ')', STATE_CLOSE_PARENTHESIS);
        PUSH(table, neutral_states[i], '[', STATE_OPEN_BRACKET);
        PUSH(table, neutral_states[i], ']', STATE_CLOSE_BRACKET);
        PUSH(table, neutral_states[i], '{', STATE_OPEN_BRACE);
        PUSH(table, neutral_states[i], '}', STATE_CLOSE_BRACE);
            
        PUSH(table, neutral_states[i], '"', STATE_STRING_LITERAL);
        PUSH(table, STATE_STRING_LITERAL, '"', STATE_STRING_LITERAL_END);
        PUSH(table, STATE_STRING_LITERAL, '\\', STATE_STRING_LITERAL_ESCAPE);
            
        PUSH(table, neutral_states[i], '\'', STATE_CHAR_LITERAL);
        PUSH(table, STATE_CHAR_LITERAL, '\'', STATE_CHAR_LITERAL_END);
        PUSH(table, STATE_CHAR_LITERAL, '\\', STATE_CHAR_LITERAL_ESCAPE);
        
        PUSH(table, STATE_LINE_COMMENT, '\n', STATE_START);

        PUSH(table, STATE_BLOCK_COMMENT, '*', STATE_BLOCK_COMMENT_ASTERISK);
        PUSH(table, STATE_BLOCK_COMMENT_ASTERISK, '*', STATE_BLOCK_COMMENT_ASTERISK);
        PUSH(table, STATE_BLOCK_COMMENT_ASTERISK, '/', STATE_START);

        PUSH(table, neutral_states[i], '/', STATE_SLASH);
        PUSH(table, STATE_SLASH, '=', STATE_SLASH_EQUALS);
        PUSH(table, STATE_SLASH, '/', STATE_LINE_COMMENT);
        PUSH(table, STATE_SLASH, '*', STATE_BLOCK_COMMENT);

        PUSH(table, neutral_states[i], ';', STATE_SEMICOLON);
        PUSH(table, neutral_states[i], '<', STATE_LESSER);
        PUSH(table, neutral_states[i], '>', STATE_GREATER);
        PUSH(table, STATE_LESSER, '<', STATE_SHIFT_LEFT);
        PUSH(table, STATE_GREATER, '>', STATE_SHIFT_RIGHT);
        PUSH(table, STATE_LESSER, '=', STATE_LESSER_EQUALS);
        PUSH(table, STATE_GREATER, '=', STATE_GREATER_EQUALS);
        PUSH(table, STATE_SHIFT_LEFT, '=', STATE_SHIFT_LEFT_EQUALS);
        PUSH(table, STATE_SHIFT_RIGHT, '=', STATE_SHIFT_RIGHT_EQUALS);
        PUSH(table, neutral_states[i], '-', STATE_MINUS);
        PUSH(table, STATE_MINUS, '-', STATE_MINUS_MINUS);
        PUSH(table, STATE_MINUS, '=', STATE_MINUS_EQUALS);
        PUSH(table, STATE_MINUS, '>', STATE_MINUS_GREATER);
        PUSH(table, neutral_states[i], '+', STATE_PLUS);
        PUSH(table, STATE_PLUS, '+', STATE_PLUS_PLUS);
        PUSH(table, STATE_PLUS, '=', STATE_PLUS_EQUALS);
        PUSH(table, neutral_states[i], '*', STATE_ASTERISK);
        PUSH(table, STATE_ASTERISK, '=', STATE_ASTERISK_EQUALS);
        PUSH(table, neutral_states[i], '&', STATE_AND);
        PUSH(table, STATE_AND, '&', STATE_AND_AND);
        PUSH(table, STATE_AND, '=', STATE_AND_EQUALS);
        PUSH(table, neutral_states[i], '|', STATE_OR);
        PUSH(table, STATE_OR, '|', STATE_OR_OR);
        PUSH(table, STATE_OR, '=', STATE_OR_EQUALS);
        PUSH(table, neutral_states[i], '%', STATE_PERCENT);
        PUSH(table, STATE_PERCENT, '=', STATE_PERCENT_EQUALS);
        PUSH(table, neutral_states[i], '^', STATE_HAT);
        PUSH(table, STATE_HAT, '=', STATE_HAT_EQUALS);
        PUSH(table, neutral_states[i], ',', STATE_COMMA);
        PUSH(table, neutral_states[i], '=', STATE_EQUALS);
        PUSH(table, STATE_EQUALS, '=', STATE_EQUALS_EQUALS);
        PUSH(table, neutral_states[i], '!', STATE_EXCLAMATION_MARK);
        PUSH(table, STATE_EXCLAMATION_MARK, '=', STATE_EXCLAMATION_MARK_EQUALS);
        PUSH(table, neutral_states[i], ':', STATE_COLON);
        PUSH(table, neutral_states[i], '~', STATE_TILDE);
        PUSH(table, neutral_states[i], '?', STATE_QUESTION_MARK);

        PUSH(table, neutral_states[i], ' ', STATE_WHITESPACE);
        PUSH(table, neutral_states[i], '\t', STATE_WHITESPACE);
        PUSH(table, neutral_states[i], '\r', STATE_WHITESPACE);
        PUSH(table, neutral_states[i], '\n', STATE_WHITESPACE);

        // to remove
        PUSH(table, neutral_states[i], '#', STATE_START);
        PUSH(table, neutral_states[i], '\\', STATE_START);
    }
}

static inline Token* PushToken(Token* tokens, uint64_t* tokenCount, uint64_t* tokenCapacity, Token token) {
    if (*tokenCount >= *tokenCapacity) {
        *tokenCapacity *= 2;
        tokens = realloc(tokens, *tokenCapacity * sizeof(Token));

        if (!tokens) {
            printf("[ERROR] Failed to reallocate %zu bytes for tokens\n", *tokenCapacity * sizeof(Token));
            return NULL;
        }
    }

    tokens[*tokenCount] = token;
    (*tokenCount)++;

    return tokens;
}

Token* Tokenize(char* data, uint64_t dataSize, DFATable table, uint64_t* tokenCount) {
    uint64_t tokenCapacity = *tokenCount > 0 ? *tokenCount : 64;
    *tokenCount = 0;

    Token* tokens = malloc(tokenCapacity * sizeof(Token));

    DFAState state = STATE_START;

    uint64_t lastCanEmitPos = 0;
    DFAState lastCanEmitState = STATE_NONE;

    uint64_t tokenStart = 0;

    for (uint64_t i = 0; i < dataSize; i++) {
        DFAState next = table[state][(unsigned char)data[i]];

        if (next) {
            if (next == STATE_WHITESPACE) tokenStart = i;

            state = next;

            if (DFAStateToTokenTypeLookup[next]) {
                lastCanEmitState = next;
                lastCanEmitPos = i;
            }

            continue;
        }

        if (lastCanEmitState) {
            char* literal = &data[tokenStart];
            uint64_t literalLength = lastCanEmitPos - tokenStart + 1;

            Token token = {
                .type = DFAStateToTokenTypeLookup[lastCanEmitState],
                .literal = literal,
                .literalLength = literalLength,
                .line = 0,
                .column = 0,
            };

            if (token.type == TK_IDENTIFIER) token.type = GetKeyword(token.literal, token.literalLength);

            tokens = PushToken(tokens, tokenCount, &tokenCapacity, token);
            if (!tokens) return NULL;

            i = lastCanEmitPos;
            state = STATE_START;
            lastCanEmitPos = 0;
            lastCanEmitState = STATE_NONE;
            tokenStart = i + 1;

            continue;
        }
        
        printf("yikes at #%li (%c), state = %i\n", i, data[i], state);
        free(tokens);
        return NULL;
    }

    if (state == STATE_BLOCK_COMMENT || state == STATE_BLOCK_COMMENT_ASTERISK || state == STATE_STRING_LITERAL || state == STATE_CHAR_LITERAL
        || state == STATE_STRING_LITERAL_ESCAPE || state == STATE_CHAR_LITERAL_ESCAPE) {
        printf("[ERROR] Unexpected EOF, state = %i\n", state);
        free(tokens);
        return NULL;
    }

    if (lastCanEmitState) {
        char* literal = &data[tokenStart];
        uint64_t literalLength = lastCanEmitPos - tokenStart + 1;

        Token token = {
            .type = DFAStateToTokenTypeLookup[lastCanEmitState],
            .literal = literal,
            .literalLength = literalLength,
            .line = 0,
            .column = 0,
        };

        if (token.type == TK_IDENTIFIER) token.type = GetKeyword(token.literal, token.literalLength);

        tokens = PushToken(tokens, tokenCount, &tokenCapacity, token);
        if (!tokens) return NULL;
    }

    Token token = {
        .type = TK_EOF,
        .literal = &data[dataSize],
        .literalLength = 0,
        .line = 0,
        .column = 0,
    };

    tokens = PushToken(tokens, tokenCount, &tokenCapacity, token);
    if (!tokens) return NULL;

    return tokens;
}

uint64_t str_to_int(const char* str) {
    uint64_t out = 0;

    for (; *str != '\0'; str++) {
        unsigned int digit = (unsigned int)(*str - '0');

        if (digit > 9) {
            return UINT64_MAX;
        }

        if (out > (UINT64_MAX - digit) / 10) {
            return UINT64_MAX;
        }

        out = out * 10 + digit;
    }

    return out;
}

int main(int argc, char** argv) {
    if (argc <= 1) {
        printf("[ERROR] No input files\n");
        return 1;
    }

    char* path = getcwd(NULL, 0);
    path = realloc(path, strlen(path) + strlen(argv[1]) + 2);
    strcat(path, "/");
    strcat(path, argv[1]);

    uint64_t dataSize;
    char* data = OpenFile(path, &dataSize);

    if (!data) {
        printf("[ERROR] Invalid input file: \"%s\"\n", path);
        free(path);
        return 1;
    }

    free(path);

    DFATable table;
    GenerateDFATable(table);

    uint64_t tokenCount = 0;
    Token* tokens;

    uint64_t n = argc >= 3 ? str_to_int(argv[2]) : 1;

    if (n == UINT64_MAX) {
        printf("[ERROR] Not a number\n");
        free(data);
        return 1;
    }
    if (n == 0) {
        printf("[ERROR] N cannot be 0\n");
        free(data);
        return 1;
    }

    clock_t start = clock();

    for (uint32_t i = 0; i < n; i++) {
        tokenCount = 0;
        tokens = Tokenize(data, dataSize, table, &tokenCount);
        if (i < n - 1) free(tokens);
    }

    clock_t end = clock();

    free(data);

    if (!tokens) {
        printf("tokenization error\n");
        return -1;
    }

    double time = (double)(end - start) / CLOCKS_PER_SEC;

    free(tokens);
    printf("\n\n\nsize=%li bytes\ntokens=%li\n", dataSize, tokenCount);
    printf("speed=%lumb/s\n", (unsigned long)((dataSize * n) / time / 1024 / 1024));
    return 0;
}