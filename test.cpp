#include <glpk.h>
#include <iostream>

int main() {
    // 創建問題
    glp_prob *lp;
    lp = glp_create_prob();
    glp_set_prob_name(lp, "sample");
    glp_set_obj_dir(lp, GLP_MAX); // 設置為求最大化問題

    // 添加變量
    glp_add_cols(lp, 3);
    glp_set_col_name(lp, 1, "x1");
    glp_set_col_bnds(lp, 1, GLP_LO, 0.0, 0.0); // x1 >= 0
    glp_set_obj_coef(lp, 1, 10.0); // 目標函數中 x1 的係數

    glp_set_col_name(lp, 2, "x2");
    glp_set_col_bnds(lp, 2, GLP_LO, 0.0, 0.0); // x2 >= 0
    glp_set_obj_coef(lp, 2, 6.0); // 目標函數中 x2 的係數

    glp_set_col_name(lp, 3, "x3");
    glp_set_col_bnds(lp, 3, GLP_LO, 0.0, 0.0); // x3 >= 0
    glp_set_obj_coef(lp, 3, 4.0); // 目標函數中 x3 的係數

    // 添加約束
    //constraint 1
    glp_add_rows(lp, 3);
    glp_set_row_name(lp, 1, "c1");
    glp_set_row_bnds(lp, 1, GLP_FX, 100.0, 100.0); // c1: x1 + x2 + x3 = 100
    // c1: x1 + x2 + x3 = 1
    // c1: x1 + x2 + x3 = 1

    //constraint 2
    glp_set_row_name(lp, 2, "c2");
    glp_set_row_bnds(lp, 2, GLP_UP, 0.0, 600.0); // c2: 10*x1 + 4*x2 + 5*x3 <= 600
    //constraint 3
    glp_set_row_name(lp, 3, "c3");
    glp_set_row_bnds(lp, 3, GLP_UP, 0.0, 300.0); // c3: 2*x1 + 2*x2 + 6*x3 <= 300


    //constraint 4
    glp_add_rows(lp, 1);
    glp_set_row_name(lp, 4, "c4");
    glp_set_row_bnds(lp, 4, GLP_UP, 0.0, 20.0); // c4: x1 + x3 <= 20

    // 填充矩陣
    int ia[1+11], ja[1+11];
    double ar[1+11];

//  ia:第一個constraint ja:第幾個變數 ar:各個參數的係數
    ia[1] = 1, ja[1] = 1, ar[1] = 1.0; // c1: x1
    ia[2] = 1, ja[2] = 2, ar[2] = 1.0; // c1: x2
    ia[3] = 1, ja[3] = 3, ar[3] = 1.0; // c1: x3

    ia[4] = 2, ja[4] = 1, ar[4] = 10.0; // c2: 10*x1
    ia[5] = 2, ja[5] = 2, ar[5] = 4.0; // c2: 4*x2
    ia[6] = 2, ja[6] = 3, ar[6] = 5.0; // c2: 5*x3

    ia[7] = 3, ja[7] = 1, ar[7] = 2.0; // c3: 2*x1
    ia[8] = 3, ja[8] = 2, ar[8] = 2.0; // c3: 2*x2
    ia[9] = 3, ja[9] = 3, ar[9] = 6.0; // c3: 6*x3

    // 新增第四個約束 c4: x1 + x3 <= 20
    ia[10] = 4; ja[10] = 1; ar[10] = 1.0; // c4: x1
    ia[11] = 4; ja[11] = 3; ar[11] = 1.0; // c4: x3

    glp_load_matrix(lp, 11, ia, ja, ar);

    // 求解問題
    glp_simplex(lp, NULL);

    // 輸出結果
    std::cout << "Optimal value: " << glp_get_obj_val(lp) << std::endl;
    std::cout << "x1 = " << glp_get_col_prim(lp, 1) << std::endl;
    std::cout << "x2 = " << glp_get_col_prim(lp, 2) << std::endl;
    std::cout << "x3 = " << glp_get_col_prim(lp, 3) << std::endl;
    
    // 釋放資源
    glp_delete_prob(lp);
    glp_free_env();

    return 0;
}
