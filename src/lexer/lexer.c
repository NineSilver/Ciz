#include <ctype.h>
#include <stdio.h>

#include <dump.h>

#include "lexer.h"

#define isident(c) (isalpha((c)) || (c) == '_')
#define isident_later(c) (isalnum((c)) || (c) == '_' || (c) == '$')

static tok_kind_t check_token_keyword(strview_t text)
{
    if(strview_compare(text, strview_from_arr_len("proc", 4)))
        return TOK_PROC_KW;
    else if(strview_compare(text, strview_from_arr_len("do", 2)))
        return TOK_DO_KW;
    else if(strview_compare(text, strview_from_arr_len("end", 3)))
        return TOK_END_KW;
    else if(strview_compare(text, strview_from_arr_len("ret", 3)))
        return TOK_RET_KW;
    else if(strview_compare(text, strview_from_arr_len("dim", 3)))
        return TOK_DIM_KW;
    else if(strview_compare(text, strview_from_arr_len("if", 2)))
        return TOK_IF_KW;
    else if(strview_compare(text, strview_from_arr_len("else", 4)))
        return TOK_ELSE_KW;
    else if(strview_compare(text, strview_from_arr_len("Int", 3)))
        return TOK_INT_KW;
    else if(strview_compare(text, strview_from_arr_len("while", 5)))
        return TOK_WHILE_KW;
    else
        return TOK_IDENTIFIER;
}

static token_t token_create(tok_kind_t kind, strview_t text, size_t line, size_t column)
{
    return (token_t){
        .kind = kind,
        .text = text,
        .line = line,
        .column = column
    };
}

lexer_t lexer_create(strview_t src)
{
    return (lexer_t){
        .src = src,
        .pos = 0,
        .line = 1,
        .column = 1
    };
}

static char lexer_current(lexer_t* lexer)
{
    return strview_at(lexer->src, lexer->pos);
}

static char lexer_peek(lexer_t* lexer, size_t off)
{
    return strview_at(lexer->src, lexer->pos + off);
}

static void lexer_advance(lexer_t* lexer)
{
    lexer->column++;
    if(lexer_current(lexer) == '\n')
    {
        lexer->line++;
        lexer->column = 1;
    }
    lexer->pos++;
}

static inline int lexer_ended(lexer_t* lexer)
{
    return (lexer->pos >= lexer->src.len);
}

token_t lexer_next_token(lexer_t* lexer)
{
    while(isspace(lexer_current(lexer)))
        lexer_advance(lexer);

    char first =  lexer_current(lexer);
    size_t start = lexer->pos, line = lexer->line, column = lexer->column;

    if(isident(first))
    {
        while(!lexer_ended(lexer) && isident_later(lexer_current(lexer)))
            lexer_advance(lexer);

        strview_t text = strview_slice(lexer->src, start, lexer->pos);

        return token_create(check_token_keyword(text), text, line, column);
    }
    else if(isdigit(first))
    {
        while(isdigit(lexer_current(lexer))) 
            lexer_advance(lexer);

        return token_create(TOK_NUMBER, strview_slice(lexer->src, start, lexer->pos), line, column);
    }
    else switch(first)
    {
        case '\"':
            lexer_advance(lexer);
            size_t from = lexer->pos;
            while(!lexer_ended(lexer) && lexer_current(lexer) != '\"')
                lexer_advance(lexer);
            size_t end = lexer->pos;
            if(lexer_current(lexer) != '\"')
                fprintf(stderr, "WARN: unquoted string at %lu:%lu\n", line, column);
            lexer_advance(lexer);
            return token_create(TOK_STRING, strview_slice(lexer->src, from, end), line, column);

        case '(':
            lexer_advance(lexer);
            return token_create(TOK_LPAREN, strview_from_arr_len("(", 1), line, column);
        
        case ')':
            lexer_advance(lexer);
            return token_create(TOK_RPAREN, strview_from_arr_len(")", 1), line, column);

        case '+':
            lexer_advance(lexer);
            return token_create(TOK_PLUS, strview_from_arr_len("+", 1), line, column);

        case '-':
            lexer_advance(lexer);
            return token_create(TOK_MINUS, strview_from_arr_len("-", 1), line, column);
        
        case '*':
            lexer_advance(lexer);
            return token_create(TOK_STAR, strview_from_arr_len("*", 1), line, column);

        case '/':
            if(lexer_peek(lexer, 1))
            {
                while(!lexer_ended(lexer) && lexer_current(lexer) != '\n')
                    lexer_advance(lexer);
                
                return lexer_next_token(lexer);
            }
            lexer_advance(lexer);
            return token_create(TOK_SLASH, strview_from_arr_len("/", 1), line, column);

        case '=':
            lexer_advance(lexer);
            if(lexer_current(lexer) == '=')
            {
                lexer_advance(lexer);
                return token_create(TOK_EQUALS, strview_from_arr_len("==", 2), line, column);
            }
            return token_create(TOK_ASSIGN, strview_from_arr_len("=", 1), line, column);

        case ':':
            if(lexer_peek(lexer, 1) == ':')
            {
                lexer_advance(lexer);
                lexer_advance(lexer);
                return token_create(TOK_AS, strview_from_arr_len("::", 2), line, column);
            }
            break;

        case ';':
            lexer_advance(lexer);
            return token_create(TOK_SEMICOLON, strview_from_arr_len(";", 1), line, column);
        
        case '!':
            if(lexer_peek(lexer, 1) == '=')
            {
                lexer_advance(lexer);
                lexer_advance(lexer);
                return token_create(TOK_NOTEQ, strview_from_arr_len("!=", 2), line, column);
            }
            break;

        case '\0':
            return token_create(TOK_EOF, strview_from_arr_len(NULL, 0), line, column);
    }

    lexer_advance(lexer);
    return token_create(TOK_UNKNOWN, strview_from_arr_len((char[2]){first, '\0'}, 1), line, column);
}
