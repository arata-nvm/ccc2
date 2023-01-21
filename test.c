typedef struct _type1_t type1_t;
struct _type1_t {
  int member1;
  char member2;
};

typedef union _type2_t type2_t;
union _type2_t {
  int as_int;
  char as_char;
};

typedef enum _type3_t {
  ENUM_HOGE,
  ENUM_FUGA,
  ENUM_PIYO,
} type3_t;

enum _type4_t {
  ENUM_HOGE2,
};

void assert(int expect, int actual);

void assert(int expect, int actual) {
  if (expect != actual) {
    printf("[NG] %d expected, but got %d\n", expect, actual);
  } else {
    printf("[OK]\n");
  }
}

int test1() { return 0; }

int test2() {
  return 1;
  return 2;
}

int test3(int v1) { return v1; }

int test4(int v1, int v2) { return v1 + v2; }

int test5(int v1) {
  if (v1 <= 2) {
    return 1;
  } else {
    return test5(v1 - 1) + test5(v1 - 2);
  }
}

void test6(int v1, ...);

int main() {
  assert(0, 0);
  assert(42, 42);
  assert(3, 1 + 2);
  assert(13, 1 + 10 + 2);
  assert(1, 2 - 1);
  assert(2, 4 - 3 + 2 - 1);
  assert(13, 1 + 3 * 4);
  assert(27, 4 * 6 + 6 / 2);
  assert(1, 3 % 2);
  assert(6, +1 * -2 * -3);
  assert(14, 1 * 2 + 3 * 4);
  assert(20, 1 * (2 + 3) * 4);
  assert(0, 0 < 0);
  assert(1, 0 < 1);
  assert(0, 1 < 0);
  assert(1, 0 <= 0);
  assert(1, 0 <= 1);
  assert(0, 1 <= 0);
  assert(0, 0 > 0);
  assert(0, 0 > 1);
  assert(1, 1 > 0);
  assert(1, 0 >= 0);
  assert(0, 0 >= 1);
  assert(1, 1 >= 0);
  assert(1, 0 == 0);
  assert(0, 0 == 1);
  assert(0, 0 != 0);
  assert(1, 0 != 1);
  {
    int v1 = 1;
    assert(1, v1);
  }
  {
    int v2 = 1;
    int v3 = 2;
    assert(3, v2 + v3);
  }
  {
    int v4 = 1;
    int v5 = 2;
    v4 = 3;
    assert(5, v4 + v5);
  }
  assert(0, test1());
  assert(1, test2());
  {
    int v6 = 0;
    while (v6 < 10) {
      v6 = v6 + 1;
    }
    assert(10, v6);
  }
  {
    int v7 = 0;
    for (int v8 = 0; v8 <= 10; v8 = v8 + 1) {
      v7 = v7 + v8;
    }
    assert(55, v7);
  }
  {}
  assert(1, test3(1));
  assert(3, test4(1, 2));
  assert(34, test5(9));
  {
    int v9 = 1;
    int *v10 = &v9;
    assert(1, *v10);
  }
  {
    int v11 = 1;
    int *v12 = &v11;
    *v12 = 2;
    assert(2, v11);
  }
  {
    int v13 = 1;
    int *v14 = &v13;
    int **v15 = &v14;
    **v15 = 2;
    assert(2, v13);
  }
  {
    int v16 = 1;
    int v17 = 2;
    int *v18 = &v16;
    v18 = v18 + 1;
    assert(2, *v18);
  }
  {
    int v19 = 1;
    int v20 = 2;
    int *v21 = &v19;
    v21 = 1 + v21;
    assert(2, *v21);
  }
  {
    int v22 = 1;
    int v23 = 2;
    int *v24 = &v23;
    v24 = v24 - 1;
    assert(1, *v24);
  }
  {
    int v25 = 1;
    int v26 = 2;
    int *v27 = &v25;
    int *v28 = &v26;
    assert(1, v28 - v27);
  }
  {
    int v29;
    int *v30;
    int v31[2];
    assert(4, sizeof(v29));
    assert(8, sizeof(v30));
    assert(8, sizeof(v31));
  }
  {
    int v32[2];
    *v32 = 1;
    *(v32 + 1) = 2;
    assert(1, *v32);
    assert(2, *(v32 + 1));
  }
  {
    int v33[2];
    v33[0] = 1;
    v33[1] = 2;
    assert(1, v33[0]);
    assert(2, v33[1]);
  }
  {
    int v34[2];
    v34[0] = 1;
    v34[1] = 2;
    v34[0] = 1;
    assert(1, v34[0]);
    assert(2, v34[1]);
  }
  {
    int v35 = 1;
    char v36 = 2;
    assert(1, v35);
    assert(2, v36);
    assert(3, v35 + v36);
  }
  {
    char v37[2];
    v37[0] = 1;
    v37[1] = 2;
    v37[0] = 1;
    assert(1, v37[0]);
    assert(2, v37[1]);
  }
  {
    char *v38 = "A";
    assert(65, v38[0]);
    assert(0, v38[1]);
  }
  // hoge
  assert(0, 0 & 0);
  assert(0, 1 & 0);
  assert(0, 0 & 1);
  assert(1, 1 & 1);
  assert(0, 0 | 0);
  assert(1, 1 | 0);
  assert(1, 0 | 1);
  assert(1, 1 | 1);
  assert(1, !0);
  assert(0, !1);
  assert(1, (~0) & 1);
  assert(0, (~1) & 1);
  assert(0, 0 ^ 0);
  assert(1, 1 ^ 0);
  assert(1, 0 ^ 1);
  assert(0, 1 ^ 1);
  assert(4, 8 >> 1);
  assert(2, 8 >> 2);
  assert(16, 8 << 1);
  assert(32, 8 << 2);
  {
    int v39 = 10;
    v39 += 2;
    assert(12, v39);
    v39 -= 2;
    assert(10, v39);
    v39 *= 2;
    assert(20, v39);
    v39 /= 2;
    assert(10, v39);
    v39 %= 3;
    assert(1, v39);
    v39 &= 0;
    assert(0, v39);
    v39 |= 3;
    assert(3, v39);
    v39 ^= 2;
    assert(1, v39);
    v39 <<= 3;
    assert(8, v39);
    v39 >>= 2;
    assert(2, v39);
  }
  {
    0 || assert(1, 1);
    1 || assert(1, 0);
    0 && assert(1, 0);
    1 && assert(1, 1);
  }
  {
    int v40 = 0;
    while (v40 < 10) {
      v40 += 1;

      if (v40 == 5) {
        break;
      }
    }
    assert(5, v40);
  }
  {
    int v41 = 0;
    while (v41 < 10) {
      v41 += 1;
      while (1) {
        break;
      }
    }
    assert(10, v41);
  }
  {
    int v44 = 0;
    while (v44 < 10) {
      v44 += 1;

      if (v44 == 5) {
        continue;
      }
    }
    assert(10, v44);
  }
  {
    int v42 = 0;
    for (int v43 = 0; v43 < 10; v43 += 1) {
      break;
      v42 += 1;
    }

    assert(0, v42);
  }
  {
    int v45 = 0;
    for (int v46 = 0; v46 < 10; v46 += 1) {
      continue;
      v45 += 1;
    }

    assert(0, v45);
  }
  {
    int v47 = 0;

    v47 = 0;
    switch (1) {
    case 1:
      v47 += 1;
      break;
    case 2:
      v47 += 2;
      break;
    default:
      v47 += 4;
      break;
    }
    assert(1, v47);

    v47 = 0;
    switch (2) {
    case 1:
      v47 += 1;
      break;
    case 2:
      v47 += 2;
      break;
    default:
      v47 += 4;
      break;
    }
    assert(2, v47);

    v47 = 0;
    switch (3) {
    case 1:
      v47 += 1;
      break;
    case 2:
      v47 += 2;
      break;
    default:
      v47 += 4;
      break;
    }
    assert(4, v47);

    v47 = 0;
    switch (1) {
    case 1:
      v47 += 1;
    case 2:
      v47 += 2;
      break;
    default:
      v47 += 4;
      break;
    }
    assert(3, v47);
  }
  {
    struct {
      int member1;
      int member2;
    } v48;
    v48.member1 = 1;
    v48.member2 = 2;
    assert(1, v48.member1);
    assert(2, v48.member2);
  }
  {
    struct {
      int member1;
      char member2;
    } v49;
    v49.member1 = 1;
    v49.member2 = 2;
    assert(1, v49.member1);
    assert(2, v49.member2);
  }
  {
    struct _type1_t v50;
    v50.member1 = 1;
    v50.member2 = 2;
    assert(1, v50.member1);
    assert(2, v50.member2);
  }
  {
    type1_t v51;
    v51.member1 = 1;
    v51.member2 = 2;
    assert(1, v51.member1);
    assert(2, v51.member2);
  }
  {
    union {
      int as_int;
      char as_char;
    } v52;
    v52.as_int = 1;
    assert(1, v52.as_int);
    assert(1, v52.as_char);
    v52.as_int = 257;
    assert(257, v52.as_int);
    assert(1, v52.as_char);
    v52.as_char = 0;
    assert(256, v52.as_int);
    assert(0, v52.as_char);
  }
  {
    union _type2_t v53;
    v53.as_int = 1;
    assert(1, v53.as_int);
    assert(1, v53.as_char);
    v53.as_int = 257;
    assert(257, v53.as_int);
    assert(1, v53.as_char);
    v53.as_char = 0;
    assert(256, v53.as_int);
    assert(0, v53.as_char);
  }
  {
    type2_t v54;
    v54.as_int = 1;
    assert(1, v54.as_int);
    assert(1, v54.as_char);
    v54.as_int = 257;
    assert(257, v54.as_int);
    assert(1, v54.as_char);
    v54.as_char = 0;
    assert(256, v54.as_int);
    assert(0, v54.as_char);
  }
  {
    union {
      struct {
        int member1;
        int member2;
      } struct1;
      struct {
        int member1;
        char member2;
      } struct2;
    } v55;
    v55.struct1.member1 = 1;
    v55.struct1.member2 = 2;
    assert(1, v55.struct1.member1);
    assert(2, v55.struct1.member2);
    assert(1, v55.struct2.member1);
    assert(2, v55.struct2.member2);
  }
  assert(0, ENUM_HOGE);
  assert(1, ENUM_FUGA);
  assert(2, ENUM_PIYO);
  assert(0, ENUM_HOGE2);
  {
    type3_t v56 = ENUM_HOGE;
    assert(0, v56);
  }
  {
    type1_t v57;
    type1_t *v58 = &v57;
    (*v58).member1 = 1;
    (*v58).member2 = 2;
    assert(1, (*v58).member1);
    assert(2, (*v58).member2);
  }
  {
    int v59 = 0;
    assert(++v59, 1);
    assert(v59++, 1);
    assert(v59, 2);
  }
  {
    int v60 = 2;
    assert(--v60, 1);
    assert(v60--, 1);
    assert(v60, 0);
  }
  {
    type1_t v61;
    type1_t *v62 = &v61;
    v62->member1 = 1;
    v62->member2 = 2;
    assert(1, v61.member1);
    assert(2, v61.member2);
    assert(1, v62->member1);
    assert(2, v62->member2);
  }
  {
    int v63 = 0;
    switch (ENUM_HOGE) {
    case ENUM_HOGE:
      v63 += 1;
      break;
    case ENUM_FUGA:
      v63 += 2;
      break;
    case ENUM_PIYO:
      v63 += 4;
      break;
    }
    assert(1, v63);
  }
  {
    int v64 = 0;
    switch (1) {
    case 1:
    case 2:
      v64 += 1;
      break;
    case 3:
      v64 += 2;
      break;
    }
    assert(1, v64);
  }
  assert(1, sizeof(char));
  assert(4, sizeof(int));
  assert(5, sizeof(type1_t));
  assert(65, 'A');
  assert(97, 'a');
  assert(10, '\n');
  assert(92, '\\');
  assert(39, '\'');
  assert(34, '\"');

  return 0;
}
