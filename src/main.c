#include <ctype.h>
#include <fenv.h>
#include <iso646.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

typedef enum TokenType {
    TK_INVALID,

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
} TokenType;

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

static inline Token CreateKeywordToken(char* input, uint64_t inputLength) {
    Token token = {
        .type = TK_IDENTIFIER,
        .literal = input,
        .literalLength = inputLength,
    };

    switch (inputLength) {
        case 2: {
            if (input[0] == 'i' && input[1] == 'f') token.type = TK_KW_IF;

            break;
        }
        case 3: {
            if (input[0] == 'm' && input[1] == 'u' && input[2] == 't') token.type = TK_KW_MUT;
            else if (input[0] == 'f' && input[1] == 'o' && input[2] == 'r') token.type = TK_KW_FOR;

            break;
        }
        case 4: {
            if (input[0] == 'e') {
                if (input[1] == 'l' && input[2] == 's' && input[3] == 'e') token.type = TK_KW_ELSE;
                else if (input[1] == 'm' && input[2] == 'i' && input[3] == 't') token.type = TK_KW_EMIT;
                else if (input[1] == 'n' && input[2] == 'u' && input[3] == 'm') token.type = TK_KW_ENUM;
            }

            break;
        }
        case 5: {
            if (input[0] == 'w' && input[1] == 'h' && input[2] == 'i' && input[3] == 'l' && input[4] == 'e') token.type = TK_KW_WHILE;
            else if (input[0] == 'b' && input[1] == 'r' && input[2] == 'e' && input[3] == 'a' && input[4] == 'k') token.type = TK_KW_BREAK;
            else if (input[0] == 'u' && input[1] == 'n' && input[2] == 'i' && input[3] == 'o' && input[4] == 'n') token.type = TK_KW_UNION;

            break;
        }
        case 6: {
            if (input[0] == 'r' && input[1] == 'e' && input[2] == 't' && input[3] == 'u' && input[4] == 'r' && input[5] == 'n') token.type = TK_KW_RETURN;
            else if (input[0] == 's' && input[1] == 't' && input[2] == 'r' && input[3] == 'u' && input[4] == 'c' && input[5] == 't') token.type = TK_KW_STRUCT;

            break;
        }
        case 8: {
            if (input[0] == 'c' && input[1] == 'o' && input[2] == 'm' && input[3] == 'p' && input[4] == 't' && input[5] == 'i' && input[6] == 'm' && input[7] == 'e') token.type = TK_KW_COMPTIME;
            else if (input[0] == 'c' && input[1] == 'o' && input[2] == 'n' && input[3] == 't' && input[4] == 'i' && input[5] == 'n' && input[6] == 'u' && input[7] == 'e') token.type = TK_KW_CONTINUE;

            break;
        }
    }

    return token;
}

static inline Token CreateOperatorToken(char* input, uint64_t size, uint64_t* i) {
    uint64_t pos = *i;

    char c0 = input[pos];
    char c1 = pos + 1 < size ? input[pos + 1] : 0;
    char c2 = pos + 2 < size ? input[pos + 2] : 0;

    Token token = {
        .type = TK_INVALID,
        .literal = &input[pos],
        .literalLength = 1,
    };

    switch (c0) {
        case '<': {
            if (c1 == '<' && c2 == '=') {
                token.type = TK_SHIFT_LEFT_EQUAL;
                token.literalLength = 3;
            }
            else if (c1 == '<') {
                token.type = TK_SHIFT_LEFT;
                token.literalLength = 2;
            }
            else if (c1 == '=') {
                token.type = TK_LESSER_EQUAL;
                token.literalLength = 2;
            }
            else {
                token.type = TK_LESSER;
            }

            break;
        }
        case '>': {
            if (c1 == '>' && c2 == '=') {
                token.type = TK_SHIFT_RIGHT_EQUAL;
                token.literalLength = 3;
            }
            else if (c1 == '>') {
                token.type = TK_SHIFT_RIGHT;
                token.literalLength = 2;
            }
            else if (c1 == '=') {
                token.type = TK_GREATER_EQUAL;
                token.literalLength = 2;
            }
            else {
                token.type = TK_GREATER;
            }
            
            break;
        }
        case '+': {
            if (c1 == '+') {
                token.type = TK_INCREMENT;
                token.literalLength = 2;
            }
            else if (c1 == '=') {
                token.type = TK_PLUS_EQUAL;
                token.literalLength = 2;
            }
            else {
                token.type = TK_PLUS;
            }

            break;
        }
        case '-': {
            if (c1 == '-') {
                token.type = TK_DECREMENT;
                token.literalLength = 2;
            }
            else if (c1 == '=') {
                token.type = TK_MINUS_EQUAL;
                token.literalLength = 2;
            }
            else if (c1 == '>') {
                token.type = TK_ARROW;
                token.literalLength = 2;
            }
            else {
                token.type = TK_MINUS;
            }

            break;
        }
        case '&': {
            if (c1 == '&') {
                token.type = TK_LOGICAL_AND;
                token.literalLength = 2;
            }
            else if (c1 == '=') {
                token.type = TK_BITWISE_AND_EQUAL;
                token.literalLength = 2;
            }
            else {
                token.type = TK_BITWISE_AND;
            }

            break;
        }
        case '|': {
            if (c1 == '|') {
                token.type = TK_LOGICAL_OR;
                token.literalLength = 2;
            }
            else if (c1 == '=') {
                token.type = TK_BITWISE_OR_EQUAL;
                token.literalLength = 2;
            }
            else {
                token.type = TK_BITWISE_OR;
            }

            break;
        }
        case '=': {
            if (c1 == '=') {
                token.type = TK_EQUAL;
                token.literalLength = 2;
            }
            else {
                token.type = TK_ASSIGN;
            }

            break;
        }
        case '*': {
            if (c1 == '=') {
                token.type = TK_MULT_EQUAL;
                token.literalLength = 2;
            }
            else {
                token.type = TK_ASTERISK;
            }

            break;
        }
        case '/': {
            if (c1 == '=') {
                token.type = TK_DIVIDE_EQUAL;
                token.literalLength = 2;
            }
            else {
                token.type = TK_SLASH;
            }

            break;
        }
        case '^': {
            if (c1 == '=') {
                token.type = TK_BITWISE_XOR_EQUAL;
                token.literalLength = 2;
            }
            else {
                token.type = TK_BITWISE_XOR;
            }

            break;
            
        }
        case '%': {
            if (c1 == '=') {
                token.type = TK_MODULO_EQUAL;
                token.literalLength = 2;
            }
            else {
                token.type = TK_MODULO;
            }

            break;
        }
        case '!': {
            if (c1 == '=') {
                token.type = TK_NOT_EQUAL;
                token.literalLength = 2;
            }
            else {
                token.type = TK_LOGICAL_NOT;
            }

            break;
        }
        case '~': {
            token.type = TK_BITWISE_NOT;

            break;
        }
        case '.': {
            token.type = TK_PERIOD;

            break;
        }
        case '?': {
            token.type = TK_QUESTION_MARK;

            break;
        }
        case ':': {
            token.type = TK_COLON;

            break;
        }
    }

    *i += token.literalLength;

    return token;
}

static const unsigned char PunctuatorTokenTypeLookup[256] = {
    [','] = TK_COMMA,
    ['('] = TK_OPEN_PAREN,
    [')'] = TK_CLOSE_PAREN,
    ['['] = TK_OPEN_BRACKET,
    [']'] = TK_CLOSE_BRACKET,
    ['{'] = TK_OPEN_BRACE,
    ['}'] = TK_CLOSE_BRACE,
    [';'] = TK_SEMICOLON,
};

static inline Token CreatePunctuatorToken(char* input) {
    TokenType type = PunctuatorTokenTypeLookup[*input];

    Token token = {
        .type = type,
        .literal = input,
        .literalLength = 1,
    };

    return token;
}

static inline bool IsAlpha(char input) {
    return ((input | 0x20) - 'a') < 26 || input == '_';
}

static inline bool IsAlphaNumeric(char input) {
    return ((input | 0x20) - 'a') < 26 || ((unsigned)(input - '0') <= 9);
}

static inline bool IsNumeric(char input) {
    return ((unsigned)(input - '0') <= 9) || input == '.' || input == '_';
}

static const unsigned char IsOperatorLookup[256] = {
    ['+'] = true,
    ['-'] = true,
    ['*'] = true,
    ['/'] = true,
    ['&'] = true,
    ['|'] = true,
    ['~'] = true,
    ['!'] = true,
    ['^'] = true,
    ['%'] = true,
    ['='] = true,
    ['<'] = true,
    ['>'] = true,
    ['.'] = true,
    ['?'] = true,
    [':'] = true,
};

static inline bool IsOperator(char input) {
    return IsOperatorLookup[input];
}

static const unsigned char IsPunctuatorLookup[256] = {
    [','] = true,
    ['('] = true,
    [')'] = true,
    ['['] = true,
    [']'] = true,
    ['{'] = true,
    ['}'] = true,
    [';'] = true,
};

static inline bool IsPunctuator(char input) {
    return IsPunctuatorLookup[input];
}

static inline void IncrementTokenCount(uint64_t* tokenCount, uint64_t* tokenCapacity, Token** tokens) {
    (*tokenCount)++;

    if (*tokenCount >= *tokenCapacity) {
        *tokenCapacity *= 2;
        *tokens = realloc(*tokens, *tokenCapacity * sizeof(Token));
    }
}

Token* Tokenize(char* data, uint64_t dataSize, uint64_t* tokenCount) {
    Token currentToken;

    uint64_t tokenStart = 0;
    uint64_t i = 0;
    uint64_t lineStartI = 0;
    uint64_t line = 0;
    uint64_t tokenCapacity = dataSize > 16 ? dataSize / 16 : 1;

    Token* tokens = malloc(tokenCapacity * sizeof(Token));

    while (true) {
        if (i >= dataSize) {
            tokens[*tokenCount].type = TK_EOF;
            break;
        }
        else if (data[i] == '\n') {
            line++;
            lineStartI = i;
            i++;
        }
        else if (data[i] == '\"') {
            tokenStart = i;
            i++;

            while (i < dataSize && data[i] != '\"' && data[i - 1] != '\\') {
                i++;
            }

            i++;

            currentToken.literal = &data[tokenStart];
            currentToken.literalLength = i - tokenStart;
            currentToken.type = TK_STRING_LITERAL;
            currentToken.line = line;
            currentToken.column = tokenStart - lineStartI;

            tokens[*tokenCount] = currentToken;
            IncrementTokenCount(tokenCount, &tokenCapacity, &tokens);
        }
        else if (data[i] == '\'') {
            tokenStart = i;
            i++;

            while (i < dataSize && data[i] != '\'' && data[i - 1] != '\\') i++;

            i++;

            currentToken.literal = &data[tokenStart];
            currentToken.literalLength = i - tokenStart;
            currentToken.type = TK_CHAR_LITERAL;
            currentToken.line = line;
            currentToken.column = tokenStart - lineStartI;

            tokens[*tokenCount] = currentToken;
            IncrementTokenCount(tokenCount, &tokenCapacity, &tokens);
        }
        else if (data[i] == '/' && data[i + 1] == '/') {
            i += 2;

            while (i < dataSize && data[i] != '\n') i++;
        }
        else if (data[i] == '/' && data[i + 1] == '*') {
            i += 2;

            while (i < dataSize && data[i] != '*' && data[i + 1] != '/') i++;

            if (i < dataSize) i += 2;
        }
        else if (IsAlpha(data[i])) {
            tokenStart = i;
            i++;

            while (i < dataSize && IsAlphaNumeric(data[i])) i++;

            currentToken = CreateKeywordToken(&data[tokenStart], i - tokenStart);
            currentToken.line = line;
            currentToken.column = tokenStart - lineStartI;

            tokens[*tokenCount] = currentToken;
            IncrementTokenCount(tokenCount, &tokenCapacity, &tokens);
        }
        else if (IsNumeric(data[i])) {
            tokenStart = i;
            i++;

            bool isFloat = data[tokenStart] == '.';

            while (i < dataSize && data[i]) {
                if (data[i] == '.') isFloat = true;
                i++;
            }

            currentToken.type = isFloat ? TK_FLOAT_LITERAL : TK_INT_LITERAL;
            currentToken.literal = &data[tokenStart];
            currentToken.literalLength = i - tokenStart;
            currentToken.line = line;
            currentToken.column = tokenStart - lineStartI;

            tokens[*tokenCount] = currentToken;
            IncrementTokenCount(tokenCount, &tokenCapacity, &tokens);
        }
        else if (IsOperator(data[i])) {
            tokenStart = i;

            currentToken = CreateOperatorToken(data, dataSize, &i);
            currentToken.line = line;
            currentToken.column = tokenStart - lineStartI;

            tokens[*tokenCount] = currentToken;
            IncrementTokenCount(tokenCount, &tokenCapacity, &tokens);
        }
        else if (IsPunctuator(data[i])) {
            tokenStart = i;
            i++;

            currentToken = CreatePunctuatorToken(&data[tokenStart]);
            currentToken.line = line;
            currentToken.column = tokenStart - lineStartI;

            tokens[*tokenCount] = currentToken;
            IncrementTokenCount(tokenCount, &tokenCapacity, &tokens);
        }
        else {
            i++;
        }
    }

    (*tokenCount)++;

    return tokens;
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

    uint64_t size;
    char* data = OpenFile(path, &size);

    if (!data) {
        printf("[ERROR] Invalid input file: \"%s\"\n", path);
        free(path);
        return 1;
    }

    free(path);

    uint64_t tokenCount = 0;

    Token* tokens;
    uint32_t n = 1024;

    for (uint32_t i = 0; i < n; i++) {
        tokenCount = 0;
        tokens = Tokenize(data, size, &tokenCount);
        if (i < n - 1) free(tokens);
    }

    free(data);

    if (tokens == NULL) {
        printf("tokenization error\n");
        return -1;
    }

    for (uint32_t i = 0; i < tokenCount - 1; i++) {
        //printf("%.*s\n", tokens[i].literalLength, tokens[i].literal);
    }

    free(tokens);
    printf("\n\n\nsize=%li bytes\ntokens=%li\n", size, tokenCount);
    return 0;
}