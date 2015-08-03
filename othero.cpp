#include<iostream>
#include<ctime>
#include<cstdlib>

using namespace std;

/*
 * ゲーム全体のベースとなる機能をまとめたクラス
 */
class OtheroUtil {
  private:
  protected:

    // 一辺のマス数
    static const int SQUARE = 8;
    // 盤管理
    static int grid[SQUARE][SQUARE];
    // 盤の角
    static const int CORNER[4][2];

    // 絶対値出力
    int abs(int num) {
      return num > 0 ? num : -num;
    }

    void initRandom() {
      srand((unsigned int)time(NULL));
    }


    // 配列の値を一つずらします
    void slideArray(int *address, int target) {
      for (int i = SQUARE*SQUARE-1; i > target; i--) {
        *(address+i) = *(address+i-1);
      }
    }

    int fillCountArray(int *address, int max) {
      int count = 0;
      for (int i = 0; i < max; i++) {
        if (*(address+i))
          count++;
      }
      return count;
    }
  public:
    OtheroUtil() {
    }
};
int OtheroUtil::grid[SQUARE][SQUARE] = {};
const int OtheroUtil::CORNER[4][2] = {
    {0, 0}, {0, SQUARE-1}, {SQUARE-1, 0}, {SQUARE-1, SQUARE-1}
};

/*
 * オセロのプレイヤー共通事項を管理するクラス
 */
class OtheroPlayerBase : public OtheroUtil {
  private:
    // ひっくり返すのは内部処理
    int reverse(bool search_only, int *mine, int my_x, int my_y);
  protected:
    // 全方向の数
    static const int ALL_DIRECTION = 8;

    // エラーメッセージ
    static const string ERROR_MESSAGE;

    // 手番
    int order;

    static const int PLAYER1 = 1;
    static const int PLAYER2 = -1;
    static const int NONE = 0;

    // Gameset
    static int gameset; 

    // 置いた手の周囲への座標(相対)
    static const int around[ALL_DIRECTION][2];
    // その方向に置ける見込みがあるか
    static int reverseFlag[ALL_DIRECTION];

    // ひっくり返すことのできるコマの終端(自分のコマ)
    static int targetPiece[ALL_DIRECTION][2];

    // 探索
    int search(bool search_only, int *mine, int my_x, int my_y);

    // スキップの判定
    bool skipJudge(int *mine);

    // 値の初期化
    void clear();

    // 盤の表示
    void printGrid();

};
const int OtheroPlayerBase::around[ALL_DIRECTION][2] = {
  {-1, -1}, {0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}
};
int OtheroPlayerBase::reverseFlag[ALL_DIRECTION] = {};
int OtheroPlayerBase::targetPiece[ALL_DIRECTION][2] = {};
const string OtheroPlayerBase::ERROR_MESSAGE = "そこに置くことはできません";
int OtheroPlayerBase::gameset = 0;

// 周囲1マスへの探索から一方向への探索
int OtheroPlayerBase::search(bool search_only, int *mine, int my_x, int my_y) {
  bool enablePut = false;
  int x = my_x, y = my_y;
  // 全方向に探索
  for (int i = 0; i < ALL_DIRECTION; i++) {
    if (x+around[i][0]<0 || x+around[i][0]>=SQUARE
        || y+around[i][1]<0 || y+around[i][1]>=SQUARE) {
      continue;
    }
    // 敵コマが隣接しているのであれば、その方向へ探索開始
    if (-(*mine) == grid[x+around[i][0]][y+around[i][1]]) {
      x = my_x + around[i][0]*2;
      y = my_y + around[i][1]*2;
      if (x < 0 || x >= SQUARE || y < 0 || y >= SQUARE) {
        x = my_x;
        y = my_y;
        continue;
      }
      while (true) {
        if (*mine == grid[x][y]) {
          // その場所にコマを置くことを許可
          enablePut = true;
          if (!search_only)
            grid[my_x][my_y] = *mine;

          // コマをとれる方向ベクトルを指定
          reverseFlag[i] = 1;

          // 終端コマを保存
          targetPiece[i][0] = x;
          targetPiece[i][1] = y;
          break;
        } else if (-(*mine) == grid[x][y]) {
          x += around[i][0];
          y += around[i][1];
          if (x < 0 || x >= SQUARE || y < 0 || y >= SQUARE)
            break;
          continue;
        } else if (!grid[x][y]) {
          break;
        }
      }
      x = my_x;
      y = my_y;
    }
  }
  
  if (!search_only && !enablePut) {
    cout << ERROR_MESSAGE << endl;
    *mine *= -1;
  }
  return reverse(search_only, mine, my_x, my_y);
}

// 反転
int OtheroPlayerBase::reverse(bool search_only, int *mine, int my_x, int my_y) {
  int reverseCount = 0;
  int x = my_x, y = my_y; 
  int loop = 0;

  for (int i = 0; i < ALL_DIRECTION; i++) {
    if (reverseFlag[i]) {
      loop = abs(x-targetPiece[i][0]) > abs(y-targetPiece[i][1]) ? abs(x-targetPiece[i][0])-1
        :abs(y-targetPiece[i][1])-1;
      reverseCount += loop;

      if (!search_only) {
        for (int j = 0; j < loop; j++) {
          x += around[i][0];
          y += around[i][1];

          grid[x][y] = *mine;
        }
        x = my_x;
        y = my_y;
      }
    }
  }
  clear();
  return reverseCount;
}
// スキップ判定 全探索サーチ
bool OtheroPlayerBase::skipJudge(int *mine) {
  bool skip = true;
  for (int i = 0; skip && i<SQUARE; i++) {
    for (int j = 0; skip && j<SQUARE; j++) {
      if (!grid[i][j] && search(true, mine, i, j))
        skip = false;
    }
  }
  if (gameset != 1)
    gameset = 0;
  if (skip) {
    cout << "Skip!" << endl;
    *mine *= -1;
    gameset++;
  }

  return skip;
}
// 初期化
void OtheroPlayerBase::clear() {
  for (int i = 0; i < ALL_DIRECTION; i++) {
    reverseFlag[i] = 0;
    targetPiece[i][0] = 0;
    targetPiece[i][1] = 0;
  }
}
// 盤の表示
void OtheroPlayerBase::printGrid() {
  cout << endl << " ";
  for (int i = 0; i < SQUARE; i++) {
    cout << " " << i;
  }
  cout << endl;
  for (int i=0; i < SQUARE; i++) {
    cout << i << " ";
    for (int j=0; j < SQUARE; j++) {
      switch (grid[j][i]) {
        case 0:
          cout << "_ ";
          break;
        case 1:
          cout << "A ";
          break;
        case -1:
          cout << "B ";
          break;
      }
    }
    cout << endl;
  }
  cout << endl;
}

class OtheroPlayer : virtual public OtheroPlayerBase {
  protected :
    // 入力受け付け
    void inputPrompt() {
      int i;
      int x = 0, y = 0;

      if (skipJudge(&order))
        return;

      if (order == PLAYER1)
        cout << "Playerの手番です: ";

      if (!(cin >> i)) {
        cout << "数字を入力してください" << endl;
        cin.clear();
        cin.ignore();
        return;
      }

      x = i / 10;
      y = i % 10;

      if (grid[x][y] != 0 || x < 0 || x >= SQUARE || y < 0 || y >= SQUARE) {
        cout << ERROR_MESSAGE << endl;
        return;
      }

      // サーチモードOFFで実行
      search(false, &order, x, y);
      printGrid();

      order *= -1;
    }
};

/*
 * オセロのアルゴリズムをまとめたクラス
 */
class OtheroAlgorithm : public OtheroUtil {
  private:
    // 正規分布に基づいた重み付け
    int normalDist[SQUARE][SQUARE];


  protected:
    static int distArray[SQUARE*SQUARE][2];

    void makeData() {

      // 乱数を初期化
      initRandom();

      int data[SQUARE];
      for (int i = 0; i < SQUARE; i++) {
        data[i] = abs(i-(SQUARE-i-1));
      }
      for (int i = 0; i < SQUARE; i++) {
        for (int j = 0; j < SQUARE; j++) {
          normalDist[i][j] = data[i] + data[j];

          // 角の重みがずれるのを回避(加算値は適当な大きい数)
          if (normalDist[i][j] == (SQUARE-1)*2)
            normalDist[i][j] += SQUARE*2;

          normalDist[i][j] += rand() % 5 - 2;
        }
      }
      // 重み付け配列
      int w[SQUARE*SQUARE] = {};
      int cnt = 0;
      for (int i = 0; i < SQUARE; i++) {
        for (int j = 0; j < SQUARE; j++) {
          for (int k = 0; k < SQUARE*SQUARE; k++) {
            if (normalDist[i][j] > w[k]) {
              slideArray(w, k);
              w[k] = normalDist[i][j];
              break;
            }
          }
        }
      }
      int check[SQUARE*SQUARE] = {};
      for (int i = 0; i < SQUARE; i++) {
        for (int j = 0; j < SQUARE; j++) {
          for (int k = 0; k < SQUARE*SQUARE; k++) {
            if (w[k] == normalDist[i][j] && check[k] == 0) {
              distArray[k][0] = i;
              distArray[k][1] = j;
              check[k] = 1;
              break;
            }
          }
        }
      }
    }
  public:
    OtheroAlgorithm() : OtheroUtil() {
    }
};
int OtheroAlgorithm::distArray[SQUARE*SQUARE][2] = {};

/*
 * オセロAI(CPU)の動作について定義したクラス
 */
class OtheroAI : virtual public OtheroPlayerBase, public OtheroAlgorithm {
  public:
    OtheroAI() : OtheroAlgorithm() {
    }

    // CPUのターン
    void inputMachine() {
      int hit = 0;

      cout << "CPUの手番です" << endl;
      if (skipJudge(&order))
        return;

      // 分布データの作成
      makeData();

      int x, y;
      // 角を優先してとる
      for (int i = 0; !hit && i<4; i++) {
        x = distArray[i][0];
        y = distArray[i][1];

        if (!grid[x][y] && (hit = search(true, &order, x, y))) {
          search(false, &order, x, y);
        }
      }
      // 正規分布に基づいて中心により近いものを取る。
      // 角周辺は極力とらないようにする(重み付け計算値が高い)
      for (int i = SQUARE*SQUARE-1; !hit && i>3; i--) {
        x = distArray[i][0];
        y = distArray[i][1];

        // 置けるのであればコマを置く
        if (!grid[x][y] && (hit = search(true, &order, x, y))) {
          search(false, &order, x, y);
        }
      }

      order *= -1;
      printGrid();
    }
};


/*
 * オセロゲームの流れを制御するクラス
 */
class Othero : public OtheroPlayer, public OtheroAI {
  private:
    // 絶対値出力
    int abs(int num) {
      return num > 0 ? num : -num;
    }

  public:
    Othero() {
      initialize();
    }

    void initialize();
    void printDescription();

    bool isFinished();
    int whichWinner(int *counter);

};
void Othero::initialize() {

  // 初期パターン
  grid[SQUARE/2-1][SQUARE/2-1] = 1;
  grid[SQUARE/2][SQUARE/2] = 1;
  grid[SQUARE/2-1][SQUARE/2] = -1;
  grid[SQUARE/2][SQUARE/2-1] = -1;

  printDescription();
  printGrid();

  string input;
  do {
    cout << "先攻・後攻どちらがいいですか?" << endl;
    cout << "先攻の場合は'A',後攻の場合は'B'を入力してください: ";
    cin >> input;
  } while (input != "A" && input != "B");

  if (input == "A")
    order = PLAYER1;
  else if (input == "B")
    order = PLAYER2;
  
  while (gameset != 2) {
    if (isFinished())
      break;
    if (order == PLAYER1)
      inputPrompt();
    else if (order == PLAYER2)
      inputMachine();
  }

  int counter[2] = {};
  switch (whichWinner(counter)) {
    case PLAYER1:
      cout << "Player win!" << endl;
      break;
    case PLAYER2:
      cout << "CPU win!" << endl;
      break;
    case NONE:
      cout << "Draw!" << endl;
      break;
  }
  cout << counter[0] << " : " << counter[1] << endl;
}

// 説明文出力
void Othero::printDescription() {
  cout << "オセロゲームです。" << endl;
  cout << "対戦相手はAIです。" << endl;
  cout << "置きたい場所の座標を[横軸][縦軸]の順で入力します。" << endl;
  cout << "example: 横0, 縦5の場所に置きたいとき、\"05\"と指定します。" << endl;
  cout << "そこにコマを置くことができない場合は、\n"
       <<  "エラーメッセージが出力されるので、別の場所を指定してください。" << endl;
  cout << "<主要な機能>" << endl;
  cout << "- オセロ基本機能(盤表示・判定・反転・勝敗)" << endl;
  cout << "- パス自動判断機能(両者パスのとき終了)" << endl;
  cout << "- 敵AI機能" << endl;
  cout << "<AI機能>" << endl;
  cout << "- 角がとれるときは取る" << endl;
  cout << "- その他の場合、正規分布に基づく手を打つ" << endl;
}


// 終了判定
bool Othero::isFinished() {
  bool flag = true;
  for (int i = 0; i < SQUARE; i++) {
    for (int j = 0; j < SQUARE; j++) {
      if (grid[i][j] == 0)
        flag = false;
    }
  }
  return flag;
}

// 勝敗判定
// 引数に配列の先頭番地を与えることでコマの取った枚数を返す
int Othero::whichWinner(int *counter) {
  int count1 = 0, count2 = 0;
  for (int i = 0; i < SQUARE; i++) {
    for (int j = 0; j < SQUARE; j++) {
      switch (grid[i][j]) {
        case PLAYER1:
          count1++;
          break;
        case PLAYER2:
          count2++;
          break;
      }
    }
  }
  *counter = count1;
  *(counter+1) = count2;

  if (count1 > count2)
    return 1;
  else if(count2 > count1)
    return -1;
  else
    return 0;
}


/*
 * 起動元
 */
int main() {
  Othero othero;
  return 0;
}
