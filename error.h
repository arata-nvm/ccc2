#define error(msg)                                                             \
  {                                                                            \
    fprintf(stderr, "%s:%d:%s\n", __FILE__, __LINE__, msg);                    \
    exit(1);                                                                   \
  }
