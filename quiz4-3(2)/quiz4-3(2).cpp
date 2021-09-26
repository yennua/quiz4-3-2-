#define _CRT_SECURE_NO_WARNINGS

#include <bangtal>
using namespace bangtal;

#include <cstdlib>
#include <ctime>

#include <iostream>

ScenePtr scene;
ObjectPtr game_board[16], game_original[16];
ObjectPtr start, restart;

TimerPtr timer;
float animationTime = 0.01f;
int mixCount = 200;

int game = 0; //초기화 작업 중일때와 게임 진행 중일때를 구분하기 위한 변수
int challnge = 1; //도전 성공 여부 확인 변수
int blank;

int game_index(ObjectPtr piece) {
    for (int i = 0; i < 16;i++)
        if (game_board[i] == piece) return i;
    return -1; //INVALID PIECE
}

int index_to_x(int index) {
	return 320 + 160 * (index % 4);
}

int index_to_y(int index) {
	return 520 - 160 * (index / 4);
}

void game_move(int index) {
	// index <-> blank
	auto piece = game_board[index];
	game_board[index] = game_board[blank];
	game_board[index]->locate(scene, index_to_x(index), index_to_y(index));
	game_board[blank] = piece;
	game_board[blank]->locate(scene, index_to_x(blank), index_to_y(blank));
	blank = index;

	if (game == 1) mixCount++; //게임 진행중이면 횟수 증가
}

bool check_move(int index) {
	if (blank % 4 > 0 && index == blank - 1) return true;
	if (blank % 4 < 3 && index == blank + 1) return true;
	if (blank / 4 > 0 && index == blank - 4) return true;
	if (blank / 4 < 3 && index == blank + 4) return true;
	return false;
}

int random_move() {
	int index = rand() % 16;
	while (!check_move(index)) {
		index = rand() % 16;
	}
	return index;
}

void start_game() {
	mixCount = 200;

	timer->set(animationTime);
	timer->start();

	blank = 15;
	game_board[blank]->hide();

	start->hide();
	restart->hide();
}

bool check_end()
{
	for (int i = 0; i < 16;i++) {
		if (game_board[i] != game_original[i]) return false;
	}
	return true;
}

void end_game() {
	game_board[blank]->show();
	restart->show();

	char clear[100];
	if (challnge == 1) {
		sprintf_s(clear, "Clear! 슬라이드 횟수: %d 챌린지: Success!", mixCount);
	}
	else sprintf_s(clear, "Clear! 슬라이드 횟수: %d 챌린지: Fail...", mixCount);
	showMessage(clear);
}

void init_game()
{
	scene = Scene::create("퍼즐", "Images/background.png");

	auto soundT = Timer::create(21.f);
	SoundPtr sound = Sound::create("Sounds/폭탄소리.mp3");
	SoundPtr sound2 = Sound::create("Sounds/시작.mp3");
	SoundPtr sound3 = Sound::create("Sounds/게임클리어8.mp3");
	
	soundT->setOnTimerCallback([&](TimerPtr timer)->bool {
		if (game == 1) sound->play(false);
		timer->stop();
		return true;
		});

	timer = Timer::create(animationTime);
	timer->setOnTimerCallback([&](auto)->bool {
		if (game == 1) challnge = 0;
		else {
			game_move(random_move());

			mixCount--;
			if (mixCount > 0) {
				timer->set(animationTime);
				timer->start();
			}
			else if(check_end()) mixCount = 100; //처음이랑 똑같이 나오면 다시 배치
			else {
				game = 1;
				mixCount = 1;
				timer->stop();
				timer->set(120.f);
				timer->start();
				soundT->set(101.f);
				sound2->play(false);
				soundT->start();
			}
			
		}
		return true;
		});

	char path[20];
	for (int i = 0; i < 16; i++) {
		sprintf(path, "Images/%d.png", i + 1);
		game_board[i] = Object::create(path, scene, index_to_x(i), index_to_y(i));
		game_board[i]->setOnMouseCallback([&](auto piece, auto x, auto y, auto action)->bool {
			//piece --> index
			int index = game_index(piece);
			if (check_move(index)) {
				game_move(index);

				if (check_end()) {
					sound3->play(false);
					timer->stop();
					soundT->stop();
					end_game();
				}
				return true;
			}
		});
		game_original[i] = game_board[i];
	}

	start = Object::create("Images/start.png", scene, 980, 40);
	restart = Object::create("Images/restart.png", scene, 980, 40);
	restart->hide();

	start->setOnMouseCallback([&](auto, auto, auto, auto)->bool {
		start_game();
		showTimer(timer);
		return true;
	});

	restart->setOnMouseCallback([&](auto, auto, auto, auto)->bool {
		start_game();
		game = 0;
		return true;
	});

	startGame(scene);
}

int main() {
	srand((unsigned int)time(NULL));

	setGameOption(GameOption::GAME_OPTION_MESSAGE_BOX_BUTTON, false);
	setGameOption(GameOption::GAME_OPTION_INVENTORY_BUTTON, false);

	init_game();
	
	return 0;
}