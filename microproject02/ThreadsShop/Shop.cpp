#include <iostream>
#include <mutex>
#include <chrono>
#include <thread>
#include <condition_variable>
#include <queue>
#include <string>
#include <utility>

using namespace std;

// Mutex ������.
std::mutex printMutex;

// �������, ����������� � ��������.
class Product
{
public:
	// ����� ������, � ������� �������� �����.
	int departmentN;
	string productName;
	Product(int n, string name)
	{
		departmentN = n;
		productName = std::move(name);
	}
};

// ������ ��������� ��������.
const vector<Product> shopsProducts = { Product(0, "gift wrapping <Snowflake>"), Product(0, "Christmas tree <Forest>"), Product(0, "garland <Stars>"),
	Product(0, "Christmas decorations <Year of the Ox>"), Product(0, "cracker <Hooray>"), Product(0, "firework <Boom>"),
	Product(0, "sparkler <Bye, 2020!>"), Product(0, "new year's mask <Santa vs covid>"), Product(0, "stocking with gifts <Surprise>"),
	Product(0, "Santa's hat <Ohoho>"), Product(1, "salad <Olivier>"), Product(1, "salad <Herring in a fur coat>"),
	Product(1, "aspic <With horseradish>"), Product(1, "champagne <Bubbles>"), Product(1, "sausage <Salami>"),
	Product(1, "salad <With pineapple>"), Product(1, "mulled wine <Spices>"), Product(1, "cake <Snowy>"),
	Product(1, "candy <Red and white>"), Product(1, "tangerines <The smell of childhood>")
};

// ������ �������� ������� ��������.
const vector<string> departments = { "Everything for the New year", "New year's food" };

// ������ �� �������� ��������.
bool isFirstSellerStartedWork = false;
bool isSecondSellerStartedWork = false;

// �������� ��������.
class Seller
{
	// ����� �������� (0 ��� 1, ��������� � ������� ������).
	int number;

	// �������� ������� ���-��.
	void saySmth(const std::string& smth) const
	{
		std::unique_lock<std::mutex> locker(printMutex);
		cout << "Seller " << number + 1 << ": " << smth << endl;
	}
public:
	// ������ ��������� ��������.
	static vector<Seller*> sellers;
	// ������� ����������� � �� ���������.
	queue<pair<Product, int>> sellerQueue;
	// Mutex ������� �����������.
	mutex queueMutex;
	// �������� ���������� - ���������� ����������� � �������.
	condition_variable queueCheck;
	// �������� ���������� - ���������� ��� � �������.
	condition_variable buyerCheck;
	// �������� ���������� - ����� �� ����������.
	condition_variable answerCheck;
	// ����� ����������, ������� ������ �������������.
	int currentBuyer;
	// ��������� �� ������� �����.
	bool isDone = false;
	// Mutex ��������.
	mutex sellerMutex;
	
	Seller(int n)
	{
		number = n;
		currentBuyer = 0;
		srand(static_cast<unsigned>(n * n + static_cast<unsigned>(time(0))));
		sellers[number] = this;

		(number == 0) ? isFirstSellerStartedWork = true : isSecondSellerStartedWork = true;
	}

	Seller(const Seller& seller)
	{
		number = seller.number;
		currentBuyer = 0;
		sellers[number] = this;
	}

	// �������� �������� ��������.
	void startWork()
	{
		// �������� �������� �� �������� ��������.
		while (true)
		{
			// ����, ���� ������ ��� � �������.
			std::unique_lock<std::mutex> locker(sellerMutex);
			queueCheck.wait(locker, [&]() {return !sellerQueue.empty();});
			while (!sellerQueue.empty())
			{
				isDone = false;
				// ����������� ���������� ������� � �������.
				queueMutex.lock();
				int buyer = sellerQueue.front().second;
				Product currentProduct = sellerQueue.front().first;
				sellerQueue.pop();
				queueMutex.unlock();

				// ����������� � �����������.
				saySmth("Hello, Buyer " + std::to_string(buyer) + "!");

				// ������� � ��������� �����.
				int t = rand() % 3 + 1; // �� 1 �� 3 ������.
				std::this_thread::sleep_for(std::chrono::seconds(t));

				// ����� ����� ����������.
				saySmth("Here is your order: " + currentProduct.productName + "!");
				currentBuyer = buyer;
				// ����� ��������.
				isDone = true;
				// ������� ���������� ����������, �������, � ������� �����.
				buyerCheck.notify_all();
				// ��� ����� ���������� ����������� � ������ �����.
				answerCheck.wait(locker);
				// �������� � ���������� ���������� ��� ��������.
				isDone = false;
			}
		}
	}
};

// ��������.
vector<Seller*> Seller::sellers = vector<Seller*>(2);

// �������� ��������� � ������ ������.
void threadSellerFunction(int a)
{
	Seller seller = Seller(a);
	seller.startWork();
}

// ���������� ����������.
class ForgetfulBuyer
{
	// ����� ����������� ����������.
	int number;

	// ������ �������.
	queue<Product> shoppingList;

	// ���������� ������� ���-��.
	void saySmth(const std::string& smth) const
	{
		std::unique_lock<std::mutex> locker(printMutex);
		cout << "Buyer " << number << ": " << smth << endl;
	}
public:
	ForgetfulBuyer(int n)
	{
		number = n;
		srand(static_cast<unsigned>(n * n + static_cast<unsigned>(time(0))));
		// ��������� ������ �������.
		int len = rand() % 10 + 1; // �� 1 �� 10.
		for (int i = 0; i < len; i++)
		{
			shoppingList.push(shopsProducts[rand() % shopsProducts.size()]);
		}
	}

	// ���������� �������� ����������.
	void shopping()
	{
		saySmth("I'm starting the shopping!");
		// �� ����������, ���� �� ����� �� �� ������.
		while (!shoppingList.empty())
		{
			// ������� �������, ������� ���� ������.
			Product currentProduct = shoppingList.front();
			shoppingList.pop();
			// ���������� ���������, ��� ���������� ��������.
			saySmth("I want buy " + currentProduct.productName);
			// ��������, � �������� ���� ������ � �������.
			Seller* currentSeller = Seller::sellers.at(currentProduct.departmentN);

			// ���������� ����� � ������� � ������� �� ����.
			{
				std::unique_lock<std::mutex> locker(currentSeller->queueMutex);
				saySmth("I'm getting in line at the " + departments[currentProduct.departmentN]);
				currentSeller->sellerQueue.push(pair<Product, int>(currentProduct, number));
				currentSeller->queueCheck.notify_one();
			}

			// ���������� ����, ���� ��� �� �������� ��������
			std::mutex waitingQueue;
			bool isProductGot = false;
			// �� ��� ���, ���� �� ����� ������� �������.
			while (!isProductGot)
			{
				std::unique_lock<std::mutex> locker(waitingQueue);
				currentSeller->buyerCheck.wait(locker, [&]()
					{
					// ��������, ���� ��� ������� � ����� ��� �����.
						return (currentSeller->currentBuyer == number && currentSeller->isDone);
					});
				// ���������� (� ������).
				saySmth("Thank you!");
				isProductGot = true;
				// ��������� ��������, ��� ���� ����� ���������� �� ���������� ����������.
				currentSeller->answerCheck.notify_one();
			}
		}
		// ���������� ����� �� �� ������!
		saySmth("I bought everything I wanted!");
	}
};

// ������� ����������.
void threadBuyerFunction(int a)
{
	ForgetfulBuyer buyer = ForgetfulBuyer(a - 1);
	while (!(isFirstSellerStartedWork && isSecondSellerStartedWork))
	{
		// ��� �������� ��������.
	}
	// ���������� �������� � ������� �� ����� ����� ��������.
	int t = rand() % 10 + 1; // �� 1 �� 10.
	std::this_thread::sleep_for(std::chrono::seconds(t));
	// �� ���������!
	buyer.shopping();
}

int main(int argc, char* argv[]) {
	srand(static_cast<unsigned>(time(0)));
	int numberOfBuyers = stoi(argv[1]);
	if (numberOfBuyers < 1 || numberOfBuyers > 10)
	{
		cout << "Number of buyers must be >= 1 and <= 10!" << endl;
		return -1;
	}
	
	cout << "Snowman's shop is opened!" << endl;
	cout << "We have 2 departments:" << endl;
	cout << "1)" + departments[0] << endl;
	cout << "2)" + departments[1] << endl;
	cout << "We are waiting for buyers!" << endl;
	cout << endl;

	// ������ ����������� � ���������.
	std::thread* threads = new std::thread[numberOfBuyers + 2];
	for (int i = 0; i < numberOfBuyers + 2; i++)
	{
		// ��������.
		if (i == 0 || i == 1)
		{
			threads[i] = std::thread(threadSellerFunction, i);
		}
		// ����������.
		else
		{
			threads[i] = std::thread(threadBuyerFunction, i);
		}
	}
	// �������, ����� ���������� ������� �� �����������.
	for (int i = 0; i < numberOfBuyers + 2; i++)
	{
		if (!(i == 0 || i == 1))
			threads[i].join();
	}
	// �������� �������� ������ ������ � ��������� ��������.
	threads[0].detach();
	threads[1].detach();
	delete[] threads;
	
	cout << endl;
	cout << "Snowman's shop is closed!" << endl;
	cout << "We look forward to seeing you tomorrow!" << endl;
	cout << "Happy New Year!" << endl;
	return 0;
}