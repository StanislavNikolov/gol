#include <iostream>
#include <ctime>
#include <random>
#include <thread>
#include <SDL2/SDL.h>

const int NINS = 1000000000; // nanoseconds in a second

int sizeX = 1366, sizeY = 768;
int drawSize = 1;
int targetFps = 60;
int threadCount = 1;
int megacellScale = 1;
bool cli = false;

bool** map;
bool** tmp;
int** renderMap;

SDL_Window* window;
SDL_Renderer* renderer;

void iterate(int colStart, int colCount)
{
	for(int x = colStart;x < colStart + colCount;++ x)
	{
		for(int y = 0;y < sizeY;++ y)
		{
			int neigbours = 0;
			if(x > 0 and map[x-1][y]) neigbours ++;
			if(x > 0 and y > 0 and map[x-1][y-1]) neigbours ++;
			if(y > 0 and map[x][y-1]) neigbours ++;
			if(x < sizeX-1 and y > 0 and map[x+1][y-1]) neigbours ++;
			if(x < sizeX-1 and map[x+1][y]) neigbours ++;
			if(x < sizeX-1 and y < sizeY-1 and map[x+1][y+1]) neigbours ++;
			if(y < sizeY-1 and map[x][y+1]) neigbours ++;
			if(x > 0 and y < sizeY-1 and map[x-1][y+1]) neigbours ++;

			if(map[x][y])
				tmp[x][y] = (neigbours == 2 or neigbours == 3);
			else
				tmp[x][y] = (neigbours == 3);
		}
	}

	for(int x = colStart;x < colStart + colCount;++ x)
		for(int y = 0;y < sizeY;++ y)
			map[x][y] = tmp[x][y];
}

void print()
{
	// clear the screan, put cursor on top
	std::cout << "\033[2J\033[1;1H";

	for(int y = 0;y < sizeY;++ y, std::cout << '\n')
		for(int x = 0;x < sizeX;++ x)
			std::cout << (map[x][y] ? '#' : ' ');
	std::cout << std::flush;
}

void sdlDraw()
{
	if(drawSize == 0)
		return;

	SDL_RenderClear(renderer);
	for(int x = 0;x < sizeX / megacellScale;++ x)
		for(int y = 0;y < sizeY / megacellScale;++ y)
			renderMap[x][y] = 0;

	for(int x = 0;x < sizeX;++ x)
		for(int y = 0;y < sizeY;++ y)
			if(map[x][y])
				renderMap[x / megacellScale][y / megacellScale] ++;

	for(int x = 0;x < sizeX / megacellScale;++ x)
	{
		for(int y = 0;y < sizeY / megacellScale;++ y)
		{
			int color = ((double)renderMap[x][y] / (double)(megacellScale * megacellScale) * 255.0);
			SDL_SetRenderDrawColor(renderer, color, color, color, 255);
			if(drawSize == 1)
			{
				SDL_RenderDrawPoint(renderer, x, y);
			}
			else
			{
				SDL_Rect rect = {x * drawSize, y  * drawSize, drawSize, drawSize};
				SDL_RenderFillRect(renderer, &rect);
			}
		}
	}
	SDL_RenderPresent(renderer);
}

void init()
{
	map = new bool* [sizeX];
	tmp = new bool* [sizeX];
	renderMap = new int* [sizeX / megacellScale];

	std::random_device seeder;
	std::default_random_engine eng(seeder());
	std::uniform_int_distribution<int> dist(0, 1);

	for(int x = 0;x < sizeX;++ x)
	{
		map[x] = new bool[sizeY];
		tmp[x] = new bool[sizeY];
		for(int y = 0;y < sizeY;++ y)
		{
			map[x][y] = dist(eng);
			tmp[x][y] = false;
		}
	}
	for(int x = 0;x < sizeX / megacellScale;++ x)
	{
		renderMap[x] = new int[sizeY / megacellScale];
	}

	if(!cli)
		SDL_CreateWindowAndRenderer(sizeX * drawSize / megacellScale, sizeY * drawSize / megacellScale, 0, &window, &renderer);
	else
		std::ios_base::sync_with_stdio(false);
}

int main(int argc, char** argv)
{
	for(int i = 0;i < argc;++ i)
	{
		if(strcmp(argv[i], "--cli") == 0)
			cli = true;
		if(strcmp(argv[i], "--scale") == 0)
			drawSize = atoi(argv[++ i]);
		if(strcmp(argv[i], "--x") == 0)
			sizeX = atoi(argv[++ i]);
		if(strcmp(argv[i], "--y") == 0)
			sizeY = atoi(argv[++ i]);
		if(strcmp(argv[i], "--fps") == 0)
			targetFps = atoi(argv[++ i]);
		if(strcmp(argv[i], "--threads") == 0)
			threadCount = atoi(argv[++ i]);
		if(strcmp(argv[i], "--megacell") == 0)
			megacellScale = atoi(argv[++ i]);
	}

	std::cout << megacellScale << std::endl;

	sizeX *= megacellScale;
	sizeY *= megacellScale;

	init();

	std::thread* threads = new std::thread[threadCount];

	if(cli) print();
	else sdlDraw();

	timespec lastTime;
	clock_gettime(CLOCK_MONOTONIC, &lastTime);
	timespec fpsCounter;
	clock_gettime(CLOCK_MONOTONIC, &fpsCounter);

	int count = 0;
	while(true)
	{
		timespec curr;
		clock_gettime(CLOCK_MONOTONIC, &curr);
		long long diff = (curr.tv_sec - lastTime.tv_sec) * NINS + curr.tv_nsec - lastTime.tv_nsec;


		if(curr.tv_sec > fpsCounter.tv_sec)
		{
			std::cout << "FPS: " << count << std::endl;
			clock_gettime(CLOCK_MONOTONIC, &fpsCounter);
			count = 0;
		}

		if(diff > NINS / targetFps)
		{
			// start all the threads
			int colsPerThread = sizeX / threadCount;
			for(int i = 0;i < threadCount-1;++ i)
				threads[i] = std::thread(iterate, i*colsPerThread, colsPerThread);

			// Make the last thread iterate to the end of the map, fix if sizeX % colsPerThread != 0
			threads[threadCount-1] = std::thread(iterate, (threadCount-1)*colsPerThread,
					sizeX - (threadCount-1)*colsPerThread);

			// and now wait for 'em to finish
			for(int i = 0;i < threadCount;++ i)
				threads[i].join();

			if(cli) print();
			else sdlDraw();

			clock_gettime(CLOCK_MONOTONIC, &lastTime);
			count ++;
		}

		SDL_Event event;
		while(SDL_PollEvent(&event) != 0)
		{
			if(event.type == SDL_QUIT)
			{
				SDL_Quit();
				return 0;
			}
		}
	}
	return 0;
}

