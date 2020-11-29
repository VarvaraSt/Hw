#include <iostream>
#include <chrono>
#include <thread>
#include <queue>
#include <string>
#include <omp.h>
//#include <windows.h>

using namespace std;

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
#pragma omp critical(print)
		{
			cout << "Student " << number << ": " << smth << endl;
		}
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
#pragma omp critical(queue)
			{
				saySmth("I solved all problems!");
				// Сдаёт работу.
				works.push(number);
			}
		}

		bool isScoreGot = false;
		// до тех пор, пока не будет получена оценка.
		while (!isScoreGot)
		{
			int a;
#pragma omp critical(scores)
			{
				a = scores[number - 1];
			}
			if (a != 0)
			{
				// если есть оценка.
				int score;
#pragma omp critical(scores)
				{
					score = scores[number - 1];
				}
				// Студент говорит, что знает свою оценку.
				saySmth("My score is " + std::to_string(score) + ".");
				isScoreGot = true;
			}
		}
	}

	void solve()
	{
		int t = rand() % 16 + 5; // от 5 до 20 секунд.
		std::this_thread::sleep_for(std::chrono::seconds(t));
		//Sleep(t * 1000);
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
#pragma omp critical(print)
		{
			cout << "Teacher: I'm starting to check student's " << n << " work!" << endl;
		}

		int t = rand() % 3 + 1; // от 1 до 3 секунд.
		std::this_thread::sleep_for(std::chrono::seconds(t));
		//Sleep(t * 1000);
		int score = rand() % 10 + 1;

#pragma omp critical(print)
		{
			cout << "Teacher: I checked student's " << n << " work! Exam score is " << score << endl;
		}

#pragma omp critical(scores)
		{
			scores[n - 1] = score;
		}
	}

	static void startExam(int numberOfStudents)
	{
		int numberOfCheckedWorks = 0;
		// до тех пор, пока не будут проверены все работы.
		while (numberOfCheckedWorks != numberOfStudents)
		{
			// если есть работы в очереди, проверять их.
			while (!works.empty())
			{
				int work;
#pragma omp critical(queue)
				{
					work = works.front();
					works.pop();
				}
				checkWork(work);
				numberOfCheckedWorks++;
				// работа проверена.
			}
		}
	}
};

int main(int argc, char* argv[]) {
	int numberOfStudents = std::stoi(argv[1]);
	if (numberOfStudents <= 0 || numberOfStudents > 100) { cout << "Wrong number of students! it should be > 0 and <= 100" << endl; return -1; }
	cout << "The exam begins! Number of students: " << numberOfStudents << endl;
	// Оценки.
	scores = std::vector<int>(numberOfStudents);
	for (int i = 0; i < numberOfStudents; i++)
	{
		scores.push_back(0);
	}

	// Потоки студентов и преподавателя.
#pragma omp parallel num_threads(numberOfStudents + 1)
	{
		auto threadN = omp_get_thread_num();
		// Поток преподавателя.
		if (threadN == 0)
		{
			Teacher::startExam(numberOfStudents);
		}
		// Потоки студентов.
		else
		{
			threadStudentFunction(omp_get_thread_num());
		}
	}

	return 0;
}