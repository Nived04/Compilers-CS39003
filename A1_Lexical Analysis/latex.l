%{

#define COMMAND 1
#define ENV_BEGIN 2
#define DISPLAY_MATH 3
#define INLINE_MATH 4

%}

WS                  [ \t\n]
ALPHABET            [a-zA-Z]
DIGIT               [0-9]
ALPHA_TEXT          {ALPHABET}+
ENV_BEGIN           \\begin{WS}*\{{ALPHA_TEXT}\}
ENV_CLOSE           \\end[{WS}]*\{{ALPHA_TEXT}\}
COMMAND             \~|\\{ALPHA_TEXT}|\\[^a-zA-Z~]
COMMENT             \%.*\n
DISPLAY_MATH        \$\$|\\\[|\\\]
INLINE_MATH         \$

%%

{WS}                    {}
{COMMENT}
{DISPLAY_MATH}          { return DISPLAY_MATH; }
{INLINE_MATH}           { return INLINE_MATH; }
{ENV_BEGIN}             { return ENV_BEGIN; }
{COMMAND}               { return COMMAND; }
.                       {}

%%

int yywrap(void) {
    return 1;
}