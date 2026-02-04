#ifndef __PARSE_H
#define __PARSE_H

#define ER_UNDEF            0 /* нет ошибок и нет результата*/
#define ER_PAIR_READY       1 /* пара готова                */
#define ER_COMPLETE         2 /* пустая пара -- хватит разберать */
#define ER_KEY_OVERFLOW     3 /* ключ слишком длянный       */
#define ER_VAL_OVERFLOW     4 /* значение слишком длинное   */
#define ER_VALUE_NOT_QUOTED 5 /* значение не в кавычках     */
#define ER_READ_XVALUE      6 /* не правильная hex-запись   */
#define ER_BREAK_VALUE      7 /* значение оборвалось        */

#define EC_INIT        0 /* разбор не начат мы перед ключом */
#define EC_DONE        1 /* разбор пары закончен            */
#define EC_READ_KEY    2 /* читаем ключ                     */
#define EC_KEY_DONE    3 /* ключ закончился пропускаем проблы и '=' */
#define EC_READ_VALUE  9 /* читаем незаквоченую строку      */
#define EC_READ_QVALUE 4 /* читаем заквоченое значение      */
#define EC_READ_SVALUE 5 /* был прочитан слеш               */
#define EC_READ_XVALUE 6 /* был прочитан \x                 */
#define EC_READ_YVALUE 7 /* был прочитан \xN и текущий символ завершнает \xnn */
#define EC_VALUE_DONE  8 /* чтение значения закончено       */

#define KEY_SIZE 32
#define VAL_SIZE 256

typedef struct eater_ctx_ {
    int state;
    int error_code;
    char key[KEY_SIZE];
    int  key_len;
    char val[VAL_SIZE];
    int  val_len;
    char hex_char;
} eater_ctx;

void eater_init(eater_ctx * ctx);
int eater_uppend(eater_ctx * ctx, char * s, int s_len);
void eater_dump(eater_ctx * ctx);

#endif
