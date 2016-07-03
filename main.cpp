#include <iostream>
#include <ctime>
#include <random>

const int sizeX = 40, sizeY = 40;
const int NINS = 1000000000; // nanosecond in a second

bool map[sizeX][sizeY];
bool tmp[sizeX][sizeY];

void iterate()
{
	for(int x = 0;x < sizeX;++ x)
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

			tmp[x][y] = false;
			if(map[x][y])
				tmp[x][y] = (neigbours == 2 or neigbours == 3);
			else
				tmp[x][y] = (neigbours == 3);
		}
	}

	for(int x = 0;x < sizeX;++ x)
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

void init()
{
	std::random_device seeder;
	std::default_random_engine eng(seeder());
	std::uniform_int_distribution<int> dist(0, 1);

	for(int x = 0;x < sizeX;++ x)
		for(int y = 0;y < sizeY;++ y)
			map[x][y] = dist(eng);
}

int main()
{
	std::ios_base::sync_with_stdio(false);
	init();

	timespec lastTime;
	clock_gettime(CLOCK_MONOTONIC, &lastTime);

	while(true)
	{
		timespec curr;
		clock_gettime(CLOCK_MONOTONIC, &curr);
		long long diff = (curr.tv_sec - lastTime.tv_sec) * NINS + curr.tv_nsec - lastTime.tv_nsec;
		if(diff > NINS / 5)
		{
			lastTime = curr;
			print();
			iterate();
		}
	}
	return 0;
}

