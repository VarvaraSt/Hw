#include <iostream>
#include <mutex>
#include <chrono>
#include <thread>
#include <condition_variable>
#include <queue>
#include <string>
#include <utility>

using namespace std;

// Mutex вывода.
std::mutex printMutex;

// Продукт, продающийся в магазине.
class Product
{
public:
	// Номер отдела, в котором продаётся товар.
	int departmentN;
	string productName;
	Product(int n, string name)
	{
		departmentN = n;
		productName = std::move(name);
	}
};

// Список продуктов магазина.
const vector<Product> shopsProducts = { Product(0, "gift wrapping <Snowflake>"), Product(0, "Christmas tree <Forest>"), Product(0, "garland <Stars>"),
	Product(0, "Christmas decorations <Year of the Ox>"), Product(0, "cracker <Hooray>"), Product(0, "firework <Boom>"),
	Product(0, "sparkler <Bye, 2020!>"), Product(0, "new year's mask <Santa vs covid>"), Product(0, "stocking with gifts <Surprise>"),
	Product(0, "Santa's hat <Ohoho>"), Product(1, "salad <Olivier>"), Product(1, "salad <Herring in a fur coat>"),
	Product(1, "aspic <With horseradish>"), Product(1, "champagne <Bubbles>"), Product(1, "sausage <Salami>"),
	Product(1, "salad <With pineapple>"), Product(1, "mulled wine <Spices>"), Product(1, "cake <Snowy>"),
	Product(1, "candy <Red and white>"), Product(1, "tangerines <The smell of childhood>")
};

// Список названий отделов магазина.
const vector<string> departments = { "Everything for the New year", "New year's food" };

// Начали ли продавцы работать.
bool isFirstSellerStartedWork = false;
bool isSecondSellerStartedWork = false;

// Продавец магазина.
class Seller
{
	// Номер продавца (0 или 1, совпадает с номером отдела).
	int number;

	// Продавец говорит что-то.
	void saySmth(const std::string& smth) const
	{
		std::unique_lock<std::mutex> locker(printMutex);
		cout << "Seller " << number + 1 << ": " << smth << endl;
	}
public:
	// Список продавцов магазина.
	static vector<Seller*> sellers;
	// ОЧередь покупателей с их покупками.
	queue<pair<Product, int>> sellerQueue;
	// Mutex очереди покупателей.
	mutex queueMutex;
	// Условная переменная - добавление покупателей в очередь.
	condition_variable queueCheck;
	// Условная переменная - покупатель ждёт в очереди.
	condition_variable buyerCheck;
	// Условная переменная - ответ от покупателя.
	condition_variable answerCheck;
	// Номер покупателя, который сейчас обслуживается.
	int currentBuyer;
	// Обработан ли текущий заказ.
	bool isDone = false;
	// Mutex продавца.
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

	// Продавец начинает работать.
	void startWork()
	{
		// Продавец работает до закрытия магазина.
		while (true)
		{
			// Спит, пока никого нет в очереди.
			std::unique_lock<std::mutex> locker(sellerMutex);
			queueCheck.wait(locker, [&]() {return !sellerQueue.empty();});
			while (!sellerQueue.empty())
			{
				isDone = false;
				// Обслуживает следующего клиента в очереди.
				queueMutex.lock();
				int buyer = sellerQueue.front().second;
				Product currentProduct = sellerQueue.front().first;
				sellerQueue.pop();
				queueMutex.unlock();

				// Здоровается с покупателем.
				saySmth("Hello, Buyer " + std::to_string(buyer) + "!");

				// Находит и пробивает товар.
				int t = rand() % 3 + 1; // от 1 до 3 секунд.
				std::this_thread::sleep_for(std::chrono::seconds(t));

				// Отдаёт товар покупателю.
				saySmth("Here is your order: " + currentProduct.productName + "!");
				currentBuyer = buyer;
				// Заказ выполнен.
				isDone = true;
				// Говорит покупателю проснуться, наконец, и забрать товар.
				buyerCheck.notify_all();
				// Ждёт когда покупатель расплатится и заберёт товар.
				answerCheck.wait(locker);
				// Перехдит к следующему покупателю или засыпает.
				isDone = false;
			}
		}
	}
};

// Продавцы.
vector<Seller*> Seller::sellers = vector<Seller*>(2);

// Создание продавцов и начало работы.
void threadSellerFunction(int a)
{
	Seller seller = Seller(a);
	seller.startWork();
}

// Забывчивый покупатель.
class ForgetfulBuyer
{
	// Номер забывчивого покупателя.
	int number;

	// Список покупок.
	queue<Product> shoppingList;

	// Покупатель говорит что-то.
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
		// Генерация списка покупок.
		int len = rand() % 10 + 1; // от 1 до 10.
		for (int i = 0; i < len; i++)
		{
			shoppingList.push(shopsProducts[rand() % shopsProducts.size()]);
		}
	}

	// Покупатель начинает закупаться.
	void shopping()
	{
		saySmth("I'm starting the shopping!");
		// Он закупается, пока не купит всё из списка.
		while (!shoppingList.empty())
		{
			// Текущий продукт, который надо купить.
			Product currentProduct = shoppingList.front();
			shoppingList.pop();
			// Покупатель объявляет, что собирается покупать.
			saySmth("I want buy " + currentProduct.productName);
			// Продавец, к которому надо встать в очередь.
			Seller* currentSeller = Seller::sellers.at(currentProduct.departmentN);

			// Покупатель встаёт в очередь и говорит об этом.
			{
				std::unique_lock<std::mutex> locker(currentSeller->queueMutex);
				saySmth("I'm getting in line at the " + departments[currentProduct.departmentN]);
				currentSeller->sellerQueue.push(pair<Product, int>(currentProduct, number));
				currentSeller->queueCheck.notify_one();
			}

			// Покупатель спит, пока его не разбудит продавец
			std::mutex waitingQueue;
			bool isProductGot = false;
			// до тех пор, пока не будет получен продукт.
			while (!isProductGot)
			{
				std::unique_lock<std::mutex> locker(waitingQueue);
				currentSeller->buyerCheck.wait(locker, [&]()
					{
					// Проснётся, если его очередь и товар уже готов.
						return (currentSeller->currentBuyer == number && currentSeller->isDone);
					});
				// Благодарит (и платит).
				saySmth("Thank you!");
				isProductGot = true;
				// Оповещает продавца, что тому можно переходить на следующего покупателя.
				currentSeller->answerCheck.notify_one();
			}
		}
		// Покупатель купил всё из списка!
		saySmth("I bought everything I wanted!");
	}
};

// Функция покупателя.
void threadBuyerFunction(int a)
{
	ForgetfulBuyer buyer = ForgetfulBuyer(a - 1);
	while (!(isFirstSellerStartedWork && isSecondSellerStartedWork))
	{
		// Ждёт открытия магазина.
	}
	// Покупатель приходит в магазин не сразу после открытия.
	int t = rand() % 10 + 1; // От 1 до 10.
	std::this_thread::sleep_for(std::chrono::seconds(t));
	// За покупками!
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

	// Потоки покупателей и продавцов.
	std::thread* threads = new std::thread[numberOfBuyers + 2];
	for (int i = 0; i < numberOfBuyers + 2; i++)
	{
		// Продавцы.
		if (i == 0 || i == 1)
		{
			threads[i] = std::thread(threadSellerFunction, i);
		}
		// Покупатели.
		else
		{
			threads[i] = std::thread(threadBuyerFunction, i);
		}
	}
	// Ожидаем, когда покупатели закупят всё необходимое.
	for (int i = 0; i < numberOfBuyers + 2; i++)
	{
		if (!(i == 0 || i == 1))
			threads[i].join();
	}
	// Продавцы закончат работу вместе с закрытием магазина.
	threads[0].detach();
	threads[1].detach();
	delete[] threads;
	
	cout << endl;
	cout << "Snowman's shop is closed!" << endl;
	cout << "We look forward to seeing you tomorrow!" << endl;
	cout << "Happy New Year!" << endl;
	return 0;
}