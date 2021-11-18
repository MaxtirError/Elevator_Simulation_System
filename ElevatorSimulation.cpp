//clock opME(0 for M, 1 for E) opid opinfo
#include<bits/stdc++.h>
#define ClockOff 0
#define ClockRun 1
#define ClockOver 2
#define GIVEUPTIMEL 500
#define GIVEUPTIMER 1000
#define NEXTPERSONL 500
#define NEXTPERSONR 1000
enum Elevator_State {GoingUp, GoingDown, Idle};
int CallUp, CallDown;
int Clock;
void Set(int &Call, int Floor, bool type) { //设置二进制某位状态
    if((Call >> Floor & 1) != type)
        Call ^= (1 << Floor);
}
int Rand(int L, int R) {
    return rand() % (R - L + 1) + L;
}
void BinaryPut(int x) {
    for(int i = 0;i <= 4; ++i)
        putchar(x >> i & 1 ? '1' : '0');
}
class Timer {
    private :
        int StartTime, WaitTime;
    public:
        Timer() {StartTime = -1; WaitTime = 0;}
        void Start(int _WaitTime) {
            StartTime = Clock;
            WaitTime = _WaitTime;
        }
        int Check() {
            if(!~StartTime)
                return ClockOff;
            if(Clock - StartTime < WaitTime)
                return ClockRun;
            StartTime = -1; WaitTime = 0;
            return ClockOver;
        }
        void Kill() {
            StartTime = -1;
            WaitTime = 0;
        }
};
class Person {
    friend class ElevatorSystem;
    private :
        int InFloor, OutFloor;
        bool InWait;
        Timer GiveUp;
    public:
        int id; // 方便输出
        void M1(int _id) { //generate data
        //Clock opME=0 opid=1 id
            id = _id;
            InFloor = Rand(0, 4);
            do {
                OutFloor = Rand(0, 4);
            }while(OutFloor == InFloor);
            GiveUp.Start(Rand(GIVEUPTIMEL, GIVEUPTIMER));
            InWait = true;
            printf("%d %d %d %d %d %d\n", Clock, 0, 1, InFloor, id, InFloor > OutFloor);
        }
        int M4() {
            if(GiveUp.Check() == ClockOver) {
                printf("%d %d %d %d %d\n", Clock, 0, 4, InFloor, id);
                return true;
            }
            return false;
        }
        int M5(int Elevator_id) {
            printf("%d %d %d %d %d %d\n", Clock, 0, 5, InFloor, id, Elevator_id);
            InWait = false;
            return OutFloor;
        }
        void M6(int Elevator_id) {
            printf("%d %d %d %d %d %d\n", Clock, 0, 6, OutFloor, id, Elevator_id);
            InWait = false;
        }
};
class PersonQueueLink {
    public:
        Person *Data;
        PersonQueueLink *next;
        PersonQueueLink(Person *p = NULL) {
            Data = p;
            next = NULL;
        }
};
class PersonQueue {
    friend class Elevator;
    friend class ElevatorSystem;
    private:
        PersonQueueLink Head, *Tail;
    public:
        PersonQueue() {
            Tail = &Head;
        }
        void Push(Person *p) {
            Tail->next = new PersonQueueLink(p);
            Tail->next->Data = p;
            Tail = Tail->next;
            Tail->next = NULL;
        }
        Person *Front() {
            return Head.next->Data;
        }
        bool Empty() {
            return Head.next == NULL;
        }
        bool Pop() {
            if(Empty()) return false;
            Head.next = Head.next->next;
            if(Head.next == NULL) 
                Tail = &Head;
            return true;   
        }
        void Check() {
            PersonQueueLink *p = Head.next, *pre = &Head;
            for(;p != NULL;) {
                if(p->Data->M4()) {
                    pre->next = p->next;
                    p = pre->next;
                }
                else {
                    pre = p;
                    p = p->next;
                }
            }
            Tail = pre;
            assert(Tail != NULL);
        }
        void OutPut(bool updown, int Floor) {
            PersonQueueLink *p = Head.next;
            for(;p != NULL; p = p->next) {
                printf(updown ? "Up" : "Down");
                printf(" Floor:%d Personid:%d\n", Floor, p->Data->id);
            }
        }
}WaitQueue[2][5];// Down 0 Up 1
class Elevator {
    friend class ElevatorSystem;
    public:
        Elevator_State State; //电梯当前状态
        int Floor; //电梯当前位置
        int D1; //D1 0 除非有人进入和离开
        int CallCar; //5个目标层按钮, 使用状态压缩
        int Elevator_id;
        Person *InsideStack[5][1001];
        int StackTop[5]; //栈顶
        void OutPut() {
            printf("State:%s\n", State == GoingUp ? "GoingUp" : (State == GoingDown ? "GoingDown" : "Idle"));
            printf("CurrentFloor:%d\n", Floor);
            printf("ButtonState:");
            BinaryPut(CallCar); 
            puts("");
            printf("PersonState:\n");
            for(int i = 0;i <= 4; ++i)
                for(int j = 1;j <= StackTop[i]; ++j)
                    printf("OutFloor:%d Person:%d\n", i, InsideStack[i][j]->id);
            printf("IfOpenDoor:%d\n", D1);
        }
    private:
        Timer Wait, Open, Close, Move_Pre, MoveUp, MoveDown, Decelerate, ReCall;
        Elevator() { 
            Elevator_id = 0;
            State = Idle;
            Floor = 1;
            D1 = 0;
            CallCar = 0;
            memset(StackTop, 0, sizeof(StackTop));
        }
        void E1() { //等候
            assert (State == Idle);
            if(CallCar || CallUp || CallDown) // 有人按电梯
                Controler(); //制动
        }
        void E2() {//回城
            if(Floor == 1)
                return ;
            if(Floor < 1) {
                ReCall.Start(15 + 51 + 14);
                printf("%d %d %d %d %d\n", Clock, 1, 2, 15 + 51 + 14, Elevator_id);
            }
            else {
                ReCall.Start(15 + 61 * (Floor - 1) + 23);
                printf("%d %d %d %d %d\n", Clock, 1, 2, 15 + 61 * (Floor - 1) + 23, Elevator_id);
            }
            Floor = 1;
        }
        void E3() { //开门
            printf("%d %d %d %d\n", Clock, 1, 3, Elevator_id);
            Set(CallCar, Floor, 0);
            Set(State == GoingUp ? CallUp : CallDown, Floor, 0);
            D1 = 1;
            Close.Start(76);
            Open.Start(20);
        }
        void E4() { //出入
            PersonQueue &Q = WaitQueue[State == GoingUp][Floor];
            Person **S = InsideStack[Floor];
            int &top = StackTop[Floor];
            if(top) {
                S[top--]->M6(Elevator_id);
                Open.Start(25);
            }
            else if(!Q.Empty()) {
                int outfloor = Q.Front()->M5(Elevator_id);
                Set(CallCar, outfloor, 1);
                InsideStack[outfloor][++StackTop[outfloor]] = Q.Front();
                Q.Pop();
                Open.Start(25);
            }
            else D1 = 0;
        }
        void E5() { //关门检查
            if(D1)
                Close.Start(40);
            else {
                printf("%d %d %d %d\n", Clock, 1, 5, Elevator_id);
                Move_Pre.Start(20);
            }
        }
        void E6() { //准备移动 & 改变状态
            assert (State != Idle);
            int Button_State = CallUp | CallDown | CallCar;
            int Mask_Up = ~(((1 << Floor + 1) - 1)) & Button_State; //mask 1表示高于Floor部分的状态
            int Mask_Down = ((1 << Floor) - 1) & Button_State; //mask 1表示低于Floor部分的状态
            if(State == GoingUp) {
                if(!Mask_Up) //没有高于Floor的
                    State = !Mask_Down ? Idle : GoingDown;
            }
            else {
                if(!Mask_Down)
                    State = !Mask_Up ? Idle : GoingUp;
            }
            if(State == Idle)
                Wait.Start(300);
            else if((State == GoingUp && CallUp >> Floor & 1) 
            || (State == GoingDown && CallDown >> Floor & 1)) { //改变状态后当前层有人按下出入按钮
                Set(CallUp, Floor, 0);
                Set(CallDown, Floor, 0);
                E3();
            }
            else {
                if(State == GoingDown) {
                    MoveDown.Start(23 + 61);
                    printf("%d %d %d %d %d %d %d\n", Clock, 1, 8, Floor, Floor - 1, 61, Elevator_id);
                }
                else {
                    MoveUp.Start(14 + 51);
                    printf("%d %d %d %d %d %d %d\n", Clock, 1, 7, Floor, Floor + 1, 51, Elevator_id);
                }
            }
        }
        void E7() {
            assert(State == GoingUp);
            ++Floor;
            if((CallCar >> Floor & 1) || (CallUp >> Floor & 1))
                Decelerate.Start(14);
            else {
                int Button_State = CallUp | CallDown | CallCar;
                int Mask_Up = ~(((1 << Floor + 1) - 1)) & Button_State; //mask 1表示高于Floor部分的状态
                if(!Mask_Up) {
                    //assert(CallDown >> Floor & 1);
                    State = GoingDown;
                    Decelerate.Start(14);
                }
                else {
                    printf("%d %d %d %d %d %d %d\n", Clock, 1, 7, Floor, Floor + 1, 51, Elevator_id);
                    MoveUp.Start(51);
                }
            }
        }
        void E8() {
            assert(State == GoingDown);
            --Floor;
            if((CallCar >> Floor & 1) || (CallDown >> Floor & 1))
                Decelerate.Start(23);
            else {
                int Button_State = CallUp | CallDown | CallCar;
                int Mask_Down = ((1 << Floor) - 1) & Button_State; //mask 1表示低于Floor部分的状态
                if(!Mask_Down) {
                    //assert(CallUp >> Floor & 1);
                    State = GoingUp;
                    Decelerate.Start(23);
                }
                else {
                    printf("%d %d %d %d %d %d %d\n", Clock, 1, 8, Floor, Floor - 1, 61, Elevator_id);
                    MoveDown.Start(61);
                }
            }
        }
        void Activate() {
            if(ReCall.Check() == ClockRun)
                return ;
            if(State == Idle)
                E1();
            if(MoveUp.Check() == ClockOver)
                E7();
            if(MoveDown.Check() == ClockOver)
                E8();
            if(Decelerate.Check() == ClockOver)
                E3();
            if(Open.Check() == ClockOver)
                E4();
            if(Close.Check() == ClockOver)
                E5();
            if(Move_Pre.Check() == ClockOver)
                E6();
            if(Wait.Check() == ClockOver)
                E2();
        }
        void Controler() { //制动
            assert(State == Idle);
            Wait.Kill();
            int Button_State = CallUp | CallDown;
            if(Button_State >> Floor & 1) {
                if(CallUp) State = GoingUp;
                else State = GoingDown;
                return E3();
            }
            int jUp, jDown;
            for(jUp = Floor + 1;jUp < 5; ++jUp)
                if(Button_State >> jUp & 1 )  
                    break;
            for(jDown = Floor - 1;jDown >= 0; --jDown)
                if(Button_State >> jDown & 1)  
                    break;
            if(jDown < 0 || Floor - jDown > jUp - Floor)
                State = GoingUp;
            else State = GoingDown;
            Move_Pre.Start(20);
        }
};
class ElevatorSystem {
    public :
        Elevator E1, E2;
        Person Pool[10001];
        Timer Inter;
        int PoolTop;
        void M2(Person &a) {
            Elevator_State dir = a.InFloor > a.OutFloor ? GoingDown : GoingUp;
            Set(dir == GoingUp ? CallUp : CallDown, a.InFloor, 1);
            assert(CallUp | CallDown != 0);
            WaitQueue[dir == GoingUp][a.InFloor].Push(&a);
            if(E1.Move_Pre.Check() == ClockRun && E1.Floor == a.InFloor && E1.State == dir) {
                E1.Move_Pre.Kill();
                E1.E3();
            }
            else if(E1.Move_Pre.Check() == ClockRun && E1.Floor == a.InFloor && E1.State == dir) {
                E2.Move_Pre.Kill();
                E2.E3();
            }
        }
        void Work() {
            E1.Elevator_id = 0;
            E2.Elevator_id = 1;
            int SimulateTime;
            scanf("%d", &SimulateTime);
            Inter.Start(Rand(NEXTPERSONL, NEXTPERSONR));
            for(Clock = 0;Clock <= SimulateTime; ++Clock) {
                /*if(Clock % 100 == 0) {
                    Output();
                    system("pause");
                }*/
                E1.Activate();
                E2.Activate();
                if(Inter.Check() == ClockOver) {
                    ++PoolTop;
                    Pool[PoolTop].M1(PoolTop);
                    M2(Pool[PoolTop]);
                    Inter.Start(Rand(NEXTPERSONL, NEXTPERSONR));
                }
                for(int i = 0;i <= 1; ++i)
                    for(int j = 0;j <= 4; ++j)
                        WaitQueue[i][j].Check();
            }
        }
        void Output() {
            printf("Time:%d\n", Clock);
            E1.OutPut();
            E2.OutPut();
            printf("UpButton:");
            BinaryPut(CallUp);
            puts("");
            printf("DownButton:");
            BinaryPut(CallDown);
            puts("");
            for(int i = 0;i <= 1; ++i)
                for(int j = 0;j <= 4; ++j)
                    WaitQueue[i][j].OutPut(i, j);
        }
}A;
int main() {
    srand(time(0));
    freopen("simulation.txt","w",stdout);
    A.Work();
    return 0;
}