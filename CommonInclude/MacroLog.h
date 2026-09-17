#ifdef __LOG_FILE__
    #define MACROLOG_FILE_SECTION __LOG_FILE__ ":"
#else
    #define MACROLOG_FILE_SECTION ""
#endif

#define MACROLOG_NUM_TO_STR(num)                #num
#define MACROLOG_NUM_MACRO_TO_STR(num_macro)    MACROLOG_NUM_TO_STR(num_macro)

#define LOCATION_PREFIX_STR_WITH_LABEL(label)   MACROLOG_FILE_SECTION label MACROLOG_NUM_MACRO_TO_STR(__LINE__) ": "
#define LOCATION_PREFIX_STR                     LOCATION_PREFIX_STR_WITH_LABEL("")

#define LOG_WITH_LOCATION(stream, reason_str)   fprintf(stream, LOCATION_PREFIX_STR_WITH_LABEL("%s:") "%s\n", __func__, reason_str)
