#include <map>
#include <queue>
#include <limits>
#include "raceState.hpp"

const int searchDepth = 2;

struct PlayerState {
    Point position;
    IntVec velocity;

    bool operator<(const PlayerState &ps) const {
        return
                position != ps.position ?
                position < ps.position :
                velocity < ps.velocity;
    }

    PlayerState(Point p, IntVec v) : position(p), velocity(v) {}
};

struct Candidate {
    int step;            // Steps needed to come here
    PlayerState state;        // State of the player
    Candidate *from;        // Came here from this place
    IntVec how;            //   with this acceleration
    Candidate(int t, PlayerState s, Candidate *f, IntVec h) :
            step(t), state(s), from(f), how(h) {}
};

// rs.positon 現在地　rs.velocity 現在の速度 course コースの情報
IntVec play(RaceState &rs, const Course &course) {
    //候補を格納するqueue
    queue<Candidate *> candidates;
    //たどり着けるかを記録するmap
    map<PlayerState, Candidate *> reached;
    //initialはプレイヤーの状態
    PlayerState initial(rs.position, rs.velocity);
    //step 初期のプレイヤーの状態　次のプレイヤーの状態　速度
    Candidate initialCand(0, initial, nullptr, IntVec(0, 0));
    //reached[initial]はinitialに辿りつけることを保存するmap
    reached[initial] = &initialCand;
    //最も良い候補を保存する変数
    Candidate *best = &initialCand;
    double goalTime = numeric_limits<double>::max();
    candidates.push(&initialCand);

    //幅優先探索
    do {
        Candidate *c = candidates.front();
        candidates.pop();
        //速度を9種類全てループ
        for (int cay = 1; cay != -2; cay--) {
            for (int cax = -1; cax != 2; cax++) {
                //次の速度
                IntVec nextVelo = c->state.velocity + IntVec(cax, cay);
                //次の位置
                Point nextPos = c->state.position + nextVelo;
                //step数が0　ライバルと衝突しない　コース状の障害物に衝突しない
                if ((c->step != 0 ||
                     !LineSegment(c->state.position, nextPos).goesThru(rs.oppPosition)) &&
                    !course.obstacled(c->state.position, nextPos)) {
                    //次のプレイヤーの位置、速度を次の候補変数に格納
                    PlayerState next(nextPos, nextVelo);
                    Candidate *nextCand =
                            new Candidate(c->step + 1, next, c, IntVec(cax, cay));

                    //次の候補でゴールが可能ならば
                    if (nextPos.y >= course.length) {
                        //tに今回の候補でゴールに着くまでの時間を記録
                        double t = c->step +
                                   (double) (course.length - c->state.position.y) / nextVelo.y;
                        //暫定goalTimeより小さければそれに決定
                        if (t < goalTime) {
                            best = nextCand;
                            goalTime = t;
                        }
                    } else if (reached.count(next) == 0) { //そこにたどり着くのが最初の候補であれば
                        //探索深さよりも浅く　かつ　コースをはみ出していなければ
                        if (c->step < searchDepth && nextPos.y < course.length) {
                            //次の候補に追加
                            candidates.push(nextCand);
                        }
                        //nextにたどり着けることを記録
                        reached[next] = nextCand;
                        //nextのy座標が現在のbestのy座標より大きいならbestを更新
                        if (nextPos.y > best->state.position.y) {
                            best = nextCand;
                        }
                    }
                }
            }
        }
    } while (!candidates.empty());



    if (best == &initialCand) {
        // No good move found
        // Slowing down for a while might be a good strategy
        int ax = 0, ay = 0;
        if (rs.velocity.x < 0) ax += 1;
        else if (rs.velocity.x > 0) ax -= 1;
        if (rs.velocity.y < 0) ay += 1;
        else if (rs.velocity.y > 0) ay -= 1;
        return IntVec(ax, ay);
    }
    Candidate *c = best;
    while (c->from != &initialCand) c = c->from;
    return c->how;
}

int main(int argc, char *argv[]) {
    Course course(cin);
    cout << 0 << endl;
    cout.flush();
    while (true) {
        RaceState rs(cin, course);
        IntVec accel = play(rs, course);
        cout << accel.x << ' ' << accel.y << endl;
    }
}
