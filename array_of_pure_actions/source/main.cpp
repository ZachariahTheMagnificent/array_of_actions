#include <iostream>
#include <algorithm>
#include <vector>
#include <memory>
#include <functional>
#include <string>
#include <thread>
#include <chrono>
#include <random>
#include <exception>
#include <stdexcept>
#include <limits>
#include <array>

#include <variant>

/**
 * Best way to represent an array of actions
 **/

namespace test
{
	using data_type = int;

	enum class test_type
	{
		enums,
		function_pointers,
		polymorphic_objects,
		std_function,
		std_variant
	};

	enum class action
	{
		multiply,
		divide,
		total_actions
	};

	using action_function = data_type(data_type data1, data_type data2);

	auto multiply(data_type data1, data_type data2)
	{
		return data1 * data2;
	};

	auto divide(data_type data1, data_type data2)
	{
		return data1 / data2;
	};

	class action_class
	{
	public:
		action_class() = default;
		virtual ~action_class() = default;

		virtual data_type operator()(data_type data1, data_type data2) const = 0;
	};

	class multiply_action_class : public action_class
	{
	public:
		multiply_action_class() = default;

		data_type operator()(data_type data1, data_type data2) const override
		{
			return data1 * data2;
		}
	};

	class divide_action_class : public action_class
	{
	public:
		divide_action_class() = default;

		data_type operator()(data_type data1, data_type data2) const override
		{
			return data1 / data2;
		}
	};

	constexpr auto multiply_variant = [](data_type data1, data_type data2)
	{
		return data1 * data2;
	};

	constexpr auto divide_variant = [](data_type data1, data_type data2)
	{
		return data1 / data2;
	};

	using std_action_function = std::function<action_function>;

	using action_variant = std::variant<decltype(multiply_variant), decltype(divide_variant)>;

	void start_test(int num_arguments, char *arguments[])
	{
		auto generator = [](auto begin, auto end)
		{
			return
			[
				rng_engine = std::mt19937_64{957546},
				rng_range = std::uniform_int_distribution{begin, end}
			]() mutable
			{
				return rng_range(rng_engine);
			};
		};

		auto data_generator = generator(1, 10'000);

		std::cout << "Array of actions test\n";

		// const auto chosen_test_type = test_type::std_variant;
		// const auto num_data = 100'000u;

		if(num_arguments < 3)
		{
			throw std::invalid_argument{"Not enough arguments lol"};
		}

		const auto chosen_test_type = static_cast<test_type>(std::stoull(arguments[1]));
		const auto num_data = std::stoull(arguments[2]);

		auto data1_array = std::vector<data_type>(num_data);
		auto data2_array = std::vector<data_type>(num_data);
		auto results = std::vector<data_type>(num_data);

		std::generate(data1_array.begin(), data1_array.end(), data_generator);
		std::generate(data2_array.begin(), data2_array.end(), data_generator);

		auto start = std::chrono::steady_clock::now();
		auto end = start;

		if(chosen_test_type == test_type::enums)
		{
			std::cout << "Testing for enums...\n";

			auto actions = std::vector<action>(num_data);
			std::generate(actions.begin(), actions.end(), [action_generator = generator(std::size_t{}, static_cast<std::size_t>(action::total_actions) - 1)]() mutable
			{
				return static_cast<action>(action_generator());
			});

			start = std::chrono::steady_clock::now();
			for(auto index = std::size_t{}, size = data1_array.size(); index < size; ++index)
			{
				if(actions[index] == action::multiply)
				{
					results[index] = data1_array[index] * data2_array[index];
				}
				else if(actions[index] == action::divide)
				{
					results[index] = data1_array[index] / data2_array[index];
				}
				else
				{
					throw std::runtime_error{"Generated an invalid action!"};
				}
			}
			end = std::chrono::steady_clock::now();
		}
		else if(chosen_test_type == test_type::function_pointers)
		{
			std::cout << "Testing for function pointers...\n";

			auto actions = std::vector<action_function*>(num_data);

			auto choices = std::array{multiply, divide};

			std::generate(actions.begin(), actions.end(), [choices, action_generator = generator(std::size_t{}, std::size(choices) - 1)]() mutable
			{
				return choices[action_generator()];
			});
			
			start = std::chrono::steady_clock::now();
			for(auto index = std::size_t{}, size = data1_array.size(); index < size; ++index)
			{
				results[index] = actions[index](data1_array[index], data2_array[index]);
			}
			end = std::chrono::steady_clock::now();
		}
		else if(chosen_test_type == test_type::polymorphic_objects)
		{
			std::cout << "Testing for polymorphic objects...\n";

			const auto multiply_action_object = multiply_action_class{};
			const auto divide_action_object = divide_action_class{};

			auto actions = std::vector<const action_class*>(num_data);

			auto choices = std::array{static_cast<const action_class*>(&multiply_action_object), static_cast<const action_class*>(&divide_action_object)};

			std::generate(actions.begin(), actions.end(), [choices, action_generator = generator(std::size_t{}, std::size(choices) - 1)]() mutable
			{
				return choices[action_generator()];
			});
			
			start = std::chrono::steady_clock::now();
			for(auto index = std::size_t{}, size = data1_array.size(); index < size; ++index)
			{
				results[index] = (*actions[index])(data1_array[index], data2_array[index]);
			}
			end = std::chrono::steady_clock::now();
		}
		else if(chosen_test_type == test_type::std_function)
		{
			std::cout << "Testing for standard functions...\n";

			auto actions = std::vector<std_action_function>(num_data);

			auto choices = std::array{std_action_function{[](auto data1, auto data2)
			{
				return data1 / data2;
			}}, std_action_function{[](auto data1, auto data2)
			{
				return data1 * data2;
			}}};

			std::generate(actions.begin(), actions.end(), [choices, action_generator = generator(std::size_t{}, std::size(choices) - 1)]() mutable
			{
				return choices[action_generator()];
			});
			
			start = std::chrono::steady_clock::now();
			for(auto index = std::size_t{}, size = data1_array.size(); index < size; ++index)
			{
				results[index] = actions[index](data1_array[index], data2_array[index]);
			}
			end = std::chrono::steady_clock::now();
		}
		else if(chosen_test_type == test_type::std_variant)
		{
			std::cout << "Testing for standard variant...\n";

			auto actions = std::vector<action_variant>{};

			auto choices = std::array{action_variant{divide_variant}, action_variant{multiply_variant}};

			std::generate_n(std::back_inserter(actions), num_data, [choices, action_generator = generator(std::size_t{}, std::size(choices) - 1)]() mutable
			{
				return choices[action_generator()];
			});
			
			start = std::chrono::steady_clock::now();
			for(auto index = std::size_t{}, size = data1_array.size(); index < size; ++index)
			{
				const auto data1 = data1_array[index];
				const auto data2 = data2_array[index];
				results[index] = std::visit([data1, data2](auto function)
				{
					return function(data1, data2);
				}, actions[index]);
			}
			end = std::chrono::steady_clock::now();
		}
		else
		{
			throw std::invalid_argument{"invalid test type lol"};
		}
		
		const auto duration = end - start;

		std::cout << "Completed in " << duration.count() << "ns\n";
	}
}

int main(int num_arguments, char *arguments[])
{
	try
	{
		test::start_test(num_arguments, arguments);
		return EXIT_SUCCESS;
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return EXIT_FAILURE;
	}
}