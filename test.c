struct type1_t {
  int member1;
  char member2;
};

int assert(int expect, int actual);

int assert(int expect, int actual) {
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
    struct type1_t v50;
    v50.member1 = 1;
    v50.member2 = 2;
    assert(1, v50.member1);
    assert(2, v50.member2);
  }

  return 0;
}
