
#ifndef PORTAUDIO_QA_PAQA_MACROS_H
#define PORTAUDIO_QA_PAQA_MACROS_H

extern int paQaNumPassed;
extern int paQaNumFailed;

/* You must use this macro exactly once in each test program. */
#define PAQA_INSTANTIATE_GLOBALS\
    int paQaNumPassed = 0;\
    int paQaNumFailed = 0;

/*------------------- Macros ------------------------------*/
/* Print ERROR if it fails. Tally success or failure. Odd  */
/* do-while wrapper seems to be needed for some compilers. */
#define ASSERT_TRUE(_exp) \
    do \
    { \
        if (_exp) {\
            paQaNumPassed++; \
        } \
        else { \
            printf("ERROR at %s:%d, (%s) not true\n", \
                __FILE__, __LINE__, #_exp ); \
            paQaNumFailed++; \
            goto error; \
        } \
    } while(0)

#define ASSERT_AB(_a, _b, _op, _opn) \
    do \
    { \
        int mA = (int)(_a); \
        int mB = (int)(_b); \
        if (mA _op mB) {\
            paQaNumPassed++; \
        } \
        else { \
            printf("ERROR at %s:%d, (%s) %s (%s), %d %s %d\n", \
                __FILE__, __LINE__, #_a, #_opn, #_b, mA, #_opn, mB ); \
            paQaNumFailed++; \
            goto error; \
        } \
    } while(0)

#define ASSERT_EQ(_a, _b) ASSERT_AB(_a, _b, ==, !=)
#define ASSERT_NE(_a, _b) ASSERT_AB(_a, _b, !=, ==)
#define ASSERT_GT(_a, _b) ASSERT_AB(_a, _b, >, <=)
#define ASSERT_GE(_a, _b) ASSERT_AB(_a, _b, >=, <)
#define ASSERT_LT(_a, _b) ASSERT_AB(_a, _b, <, >=)
#define ASSERT_LE(_a, _b) ASSERT_AB(_a, _b, <=, >)

#define HOPEFOR(_exp) \
    do \
    { \
        if ((_exp)) {\
            paQaNumPassed++; \
        } \
        else { \
            printf("\nERROR - 0x%x - %s for %s\n", result, Pa_GetErrorText(result), #_exp ); \
            paQaNumFailed++; \
        } \
    } while(0)

#define PAQA_PRINT_RESULT \
        printf("QA Report: %d passed, %d failed.\n", paQaNumPassed, paQaNumFailed )

#define PAQA_EXIT_RESULT \
        (((paQaNumFailed > 0) || (paQaNumPassed == 0)) ? EXIT_FAILURE : EXIT_SUCCESS)

#endif /* PORTAUDIO_QA_PAQA_MACROS_H */
