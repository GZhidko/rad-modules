#include "parse.h"
#include "log.h"
#undef DEBUG_PARSE
#ifdef DEBUG_PARSE
#include <stdio.h>
#endif

void eater_init(eater_ctx * ctx) {
    ctx->state = EC_INIT;
    ctx->error_code = ER_UNDEF;
}

void _eater_append_char_to_key(eater_ctx * ctx, char c) {
    if (ctx->key_len >= KEY_SIZE) {
        ctx->error_code = ER_KEY_OVERFLOW;
    } else {
        ctx->key[ctx->key_len++] = c;
    }
}

void _eater_append_char_to_value(eater_ctx * ctx, char c) {
    if (ctx->val_len >= VAL_SIZE) {
        ctx->error_code = ER_VAL_OVERFLOW;
    } else {
#ifdef DEBUG_PARSE
        printf("\tparser.c:\tappend=%x (%c)\n", (int)c, c);
#endif
        ctx->val[ctx->val_len++] = c;
    }
}

int _hex_digit_to_int(char c) {
    if (c <= '9') return (c - 48);
    if (c <= 'A') return (c - 65 + 10);
    return (c - 97 + 10); // if (c < 'a')
}

void _eater_append_hex_to_value(eater_ctx * ctx, char c2) {
#ifdef DEBUG_PARSE
    printf("\tparser.c:\tappend= %x (%c) %i | %x (%c) %i (hex start)\n", (int)c2, c2, _hex_digit_to_int(c2),
       (int)ctx->hex_char, ctx->hex_char, _hex_digit_to_int(ctx->hex_char));
#endif
    _eater_append_char_to_value(ctx,
                               _hex_digit_to_int(c2) +
                               _hex_digit_to_int(ctx->hex_char) * 16);
}

int _is_valid_hex_digit(char c) {
    return (('0' <= c && c <= '9') ||
            ('a' <= c && c <= 'f') ||
            ('A' <= c && c <= 'F'));
}

/* Анализирует строку s длинной s_len в контексте ctx;
   останавливается, если нашла пару A/V или произошла
   ошибка.
   Возвращает кол-во обработанных символов.
   После выхода следует обработать
   ctx->error_code;
   ER_COMPLETE -- была прочитана пустая пара (конец пакета)
   
   Распознаётся только один тип квотирования символов \xNN.
   Это квотирование работае только закавыченых строках.
   Незаковыченные строки не парсятся на предмет \x.
*/
int eater_uppend(eater_ctx * ctx, char * s, int s_len) {
    int p;
    char c;
    for (p = 0; p < s_len;) {
        c = s[p++];
#ifdef DEBUG_PARSE
        printf("\tparser.c:\tpocess=%x (%c)\tstate=%d\n", (int)c, c, ctx->state);
#endif
        switch (ctx->state) {
            case EC_DONE:
                /* done == init */
            case EC_INIT:
                /* читаем всё до ключа */
                ctx->error_code = ER_UNDEF; /* в начале сбрасываем и ошбку */
                ctx->key_len = 0;
                ctx->val_len = 0;
                switch (c) {
                    case '\n':
                        ctx->state = EC_DONE;
                        ctx->error_code = ER_COMPLETE;
                        break;
                    case ' ':
                    case '\t':
                        break;
                    default:
                        ctx->state = EC_READ_KEY;
                        _eater_append_char_to_key(ctx, c);
                        break;
                }
                break;
            case EC_READ_KEY:
                /* читаем ключ, пока он не кончится */
                switch (c) {
                    case ' ':
                    case '\t':
                    case '=':
                        ctx->state = EC_KEY_DONE;
                        break;
                    default:
                        _eater_append_char_to_key(ctx, c);
                        break;
                }
                break;
            case EC_KEY_DONE:
                /* ключ был прочитан, пропускаем всё до значения */
                switch (c) {
                    case ' ':
                    case '\t':
                    case '=':
                        break;
                    case '"':
                        ctx->state = EC_READ_QVALUE;
                        break;
                    default:
                        ctx->state = EC_READ_VALUE;
                        _eater_append_char_to_value(ctx, c);
                        break;
                }
                break;
            case EC_READ_VALUE:
                /* читаем незаквоченую строку (число) */
                switch (c) {
                    case '\n':
                        ctx->state = EC_DONE;
                        ctx->error_code = ER_PAIR_READY;
                        break;
                    default:
                        _eater_append_char_to_value(ctx, c);
                        break;
                }
                break;
            case EC_READ_QVALUE:
                /* читаем заквотированую строку */
                switch (c) {
                    case '\\':
                        ctx->state = EC_READ_SVALUE;
                        break;
                    case '"':
                        ctx->state = EC_VALUE_DONE;
                        break;
                    default:
                        _eater_append_char_to_value(ctx, c);
                        break;
                }
                break;
            case EC_READ_SVALUE:
                /* читаем 'x' после '/' */
                switch (c) {
                    case 'x':
                    case 'X':
                        ctx->state = EC_READ_XVALUE;
                        break;
                    default:
                        ctx->error_code = ER_READ_XVALUE;
                        break;
                }
                break;
            case EC_READ_XVALUE:
                /* читаем первый знак шетнадцатиричной цифры */
                if (_is_valid_hex_digit(c)) {
                    ctx->hex_char = c;
                    ctx->state = EC_READ_YVALUE;
                } else {
                    ctx->error_code = ER_READ_XVALUE;
                }
                break;
            case EC_READ_YVALUE:
                /* читаем второй знак */
                if (_is_valid_hex_digit(c)) {
                    _eater_append_hex_to_value(ctx, c);
                    ctx->state = EC_READ_QVALUE;
                } else {
                    ctx->error_code = ER_READ_XVALUE;
                }
                break;
            case EC_VALUE_DONE:
                /* должен быть перевод строки */
                switch (c) {
                    case '\n':
                        ctx->state = EC_DONE;
                        ctx->error_code = ER_PAIR_READY;
                        break;
                    default:
                        ctx->error_code = ER_BREAK_VALUE;
                        break;
                }
                break;
        }
        if (ctx->error_code != ER_UNDEF) {
            /* чтобы получился EC_DONE
               достаточно выставить определённых флаг */
            ctx->state = EC_DONE;
            break;
        }
    }
    return p;
}

void eater_dump(eater_ctx * ctx) {
    char buffkey[4000];
    char buffval[4000];
    log_escape_string(ctx->key, ctx->key_len, buffkey, 4000);
    log_escape_string(ctx->val, ctx->val_len, buffval, 4000);
    write_info("** DUMP ** : "
               "state = %d, "
               "error_code = %d, "
               "key = [%s], "
               "val = [%s], "
               "hex = [%c]",
               ctx->state,
               ctx->error_code,
               buffkey,
               buffval,
               ctx->hex_char);
}
