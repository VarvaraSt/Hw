#include <iostream>
#include <mutex>
#include <chrono>
#include <thread>
#include <condition_variable>
#include <queue>
#include <string>

using namespace std;

// Mutex вывода.
std::mutex printMutex;
// Mutex препдавателя в ожидании ответов.
std::mutex teacherMutex;
// Mutex доступа к оценкам.
std::mutex scoresLock;
// Mutex очереди на оценивание.
std::mutex queueMutex;
// Условие появления работы в очереди.
std::condition_variable queueСheck;
// Условие появления оценки.
std::condition_variable scoreCheck;
// Очередь работ.
queue<int> works;
// Список оценок.
std::vector<int> scores;

// Студент.
class Student
{
	// Номер студента.
	int number;

	// Студент говорит что-то.
	void saySmth(std::string smth)
	{
		std::unique_lock<std::mutex> locker(printMutex);
		cout << "Student " << number << ": " << smth << endl;
	}
	
public:
	Student(int n)
	{
		number = n;
		srand(static_cast<unsigned>(n * n + static_cast<unsigned>(time(0))));
	}

	// Процесс сдачи экзамена.
	void takeExam()
	{
		saySmth("I'm starting to solve!");

		solve();

		{
			std::unique_lock<std::mutex> locker(queueMutex);
			saySmth("I solved all problems!");
			// Сдаёт работу.
			works.push(number);
			queueСheck.notify_one();
		}

		std::mutex waitingScore;
		bool isScoreGot = false;
		// до тех пор, пока не будет получена оценка.
		while (!isScoreGot)
		{
			std::unique_lock<std::mutex> locker(waitingScore);
			scoreCheck.wait(locker, [&]()
				{
					scoresLock.lock();
					int a = scores[number - 1];
					scoresLock.unlock();
					return a != 0;
				});
			// если есть оценка.
			scoresLock.lock();
			int score = scores[number - 1];
			scoresLock.unlock();
			// Студент говорит, что знает свою оценку.
			saySmth("My score is " + std::to_string(score) + ".");
			isScoreGot = true;
		}
	}

	void solve()
	{
		int t = rand() % 16 + 5; // от 5 до 20 секунд.
		std::this_thread::sleep_for(std::chrono::seconds(t));
	}
};

void threadStudentFunction(int a)
{
	Student stud = Student(a);
	stud.takeExam();
}

// Преподаватель.
class Teacher
{
public:
	// Проверяет работу.
	static void checkWork(int n)
	{
		printMutex.lock();
		cout << "Teacher: I'm starting to check student's " << n << " work!" << endl;
		printMutex.unlock();

		int t = rand() % 3 + 1; // от 1 до 3 секунд.
		std::this_thread::sleep_for(std::chrono::seconds(t));
		int score = rand() % 10 + 1;

		printMutex.lock();
		cout << "Teacher: I checked student's " << n << " work! Exam score is " << score << endl;
		printMutex.unlock();

		scoresLock.lock();
		scores[n - 1] = score;
		scoresLock.unlock();
	}

	static void startExam(int numberOfStudents)
	{
		int numberOfCheckedWorks = 0;
		// до тех пор, пока не будут проверены все работы.
		while (numberOfCheckedWorks != numberOfStudents)
		{
			std::unique_lock<std::mutex> locker(teacherMutex);
			queueСheck.wait(locker, [&]() {return !works.empty();});
			// если есть работы в очереди, проверять их.
			while (!works.empty())
			{
				queueMutex.lock();
				int work = works.front();
				works.pop();
				queueMutex.unlock();
				checkWork(work);
				numberOfCheckedWorks++;
				// работа проверена.
				scoreCheck.notify_all();
			}
		}
	}
};

int main(int argc, char* argv[]) {
	int numberOfStudents = std::stoi(argv[1]);
	if (numberOfStudents <= 0 || numberOfStudents > 100) { cout << "Wrong number of students! it should be > 0 and <= 100" << endl; return -1; }
	// Оценки.
	scores = std::vector<int>(numberOfStudents);
	for (int i = 0; i < numberOfStudents; i++)
	{
		scores.push_back(0);
	}
	// Поток преподавателя.
	std::thread serverThread(Teacher::startExam, numberOfStudents);
	// Потоки учеников.
	std::thread* threads = new std::thread[numberOfStudents];
	for (int i = 0; i < numberOfStudents; i++)
	{
		threads[i] = std::thread(threadStudentFunction, i + 1);
	}
	for (int i = 0; i < numberOfStudents; i++)
	{
		threads[i].join();
	}
	delete[] threads;
	serverThread.join();
	return 0;
}