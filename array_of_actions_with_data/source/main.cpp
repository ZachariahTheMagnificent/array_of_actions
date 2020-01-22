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
 * Best way to represent an array of actions with data
 **/

namespace test
{
	enum class test_type
	{
		enums,
		function_pointers,
		polymorphic_objects,
		std_function,
		std_variant
	};

	struct character
	{
		float health;
		float attack;
	};

	enum class action_type
	{
		damage,
		heal,
		buff,
		total_actions
	};

	struct action
	{
		action_type type;
		void* data;
	};

	using action_function = character(character subject, void* data);

	struct action_function_data
	{
		action_function* function;
		void* data;
	};

	auto damage(character subject, void* data)
	{
		return character{subject.health - *static_cast<float*>(data), subject.attack};
	};

	auto heal(character subject, void* data)
	{
		return character{subject.health + *static_cast<float*>(data), subject.attack};
	};

	auto buff(character subject, void* data)
	{
		return character{subject.health, subject.attack + *static_cast<float*>(data)};
	};

	class action_class
	{
	public:
		action_class() = default;
		virtual ~action_class() = default;

		virtual character operator()(character subject) const = 0;
	};

	class damage_action_class : public action_class
	{
	public:
		damage_action_class() = default;
		damage_action_class(const float damage)
		:
		damage{damage}
		{

		};

		character operator()(character subject) const override
		{
			return character{subject.health - damage, subject.attack};
		}

	private:
		float damage;
	};

	class heal_action_class : public action_class
	{
	public:
		heal_action_class() = default;
		heal_action_class(const float heal)
		:
		heal{heal}
		{

		};

		character operator()(character subject) const override
		{
			return character{subject.health + heal, subject.attack};
		}

	private:
		float heal;
	};

	class buff_action_class : public action_class
	{
	public:
		buff_action_class() = default;
		buff_action_class(const float buff)
		:
		buff{buff}
		{

		};

		character operator()(character subject) const override
		{
			return character{subject.health, subject.attack + buff};
		}

	private:
		float buff;
	};

	constexpr auto damage_variant = [](const auto damage)
	{
		return [damage](character subject)
		{
			return character{subject.health - damage, subject.attack};
		};
	};

	constexpr auto heal_variant = [](const auto heal)
	{
		return [heal](character subject)
		{
			return character{subject.health + heal, subject.attack};
		};
	};

	constexpr auto buff_variant = [](const auto buff)
	{
		return [buff](character subject)
		{
			return character{subject.health, subject.attack + buff};
		};
	};

	using std_action_function = std::function<character(character subject)>;

	using action_variant = std::variant<decltype(damage_variant(float{})), decltype(heal_variant(float{})), decltype(buff_variant(float{}))>;

	void start_test(int num_arguments, char *arguments[])
	{
		auto generator = [](auto begin, auto end)
		{
			const auto seed = 957546;

			if constexpr(std::is_floating_point_v<decltype(begin)>)
			{
				return
				[
					rng_engine = std::mt19937_64{seed},
					rng_range = std::uniform_real_distribution{begin, end}
				]() mutable
				{
					return rng_range(rng_engine);
				};
			}
			else
			{
				return
				[
					rng_engine = std::mt19937_64{seed},
					rng_range = std::uniform_int_distribution{begin, end}
				]() mutable
				{
					return rng_range(rng_engine);
				};
			}
		};

		auto data_generator = generator(1.0f, 10'000.0f);

		std::cout << "Array of actions with data test\n";

		// const auto chosen_test_type = test_type::function_pointers;
		// const auto num_data = 1'000'000u;

		if(num_arguments < 3)
		{
			throw std::invalid_argument{"Not enough arguments lol"};
		}

		const auto chosen_test_type = static_cast<test_type>(std::stoull(arguments[1]));
		const auto num_data = std::stoull(arguments[2]);

		auto characters = std::vector<character>(num_data);
		auto results = std::vector<character>(num_data);

		std::generate(characters.begin(), characters.end(), [&data_generator]() mutable
		{
			return character{data_generator(), data_generator()};
		});

		auto start = std::chrono::steady_clock::now();
		auto end = start;

		if(chosen_test_type == test_type::enums)
		{
			std::cout << "Testing for enums...\n";

			auto actions = std::vector<action>();
			std::generate_n(std::back_inserter(actions), num_data, [&data_generator, action_generator = generator(std::size_t{}, static_cast<std::size_t>(action_type::total_actions) - 1)]() mutable
			{
				return action{static_cast<action_type>(action_generator()), new (std::malloc(sizeof(float))) float(data_generator())};
			});

			start = std::chrono::steady_clock::now();
			for(auto index = std::size_t{}, size = characters.size(); index < size; ++index)
			{
				if(actions[index].type == action_type::buff)
				{
					results[index] = character{characters[index].health - *static_cast<float*>(actions[index].data), characters[index].attack};
				}
				else if(actions[index].type == action_type::damage)
				{
					results[index] = character{characters[index].health + *static_cast<float*>(actions[index].data), characters[index].attack};
				}
				else if(actions[index].type == action_type::heal)
				{
					results[index] = character{characters[index].health, characters[index].attack + *static_cast<float*>(actions[index].data)};
				}
				else
				{
					throw std::runtime_error{"Generated an invalid action!"};
				}
			}
			end = std::chrono::steady_clock::now();

			for(auto& current_action : actions)
			{
				std::free(current_action.data);
			}
		}
		else if(chosen_test_type == test_type::function_pointers)
		{
			std::cout << "Testing for function pointers...\n";

			auto actions = std::vector<action_function_data>();

			using choice_generator = std::function<action_function_data()>;
			auto choices = std::array
			{
				choice_generator{[&data_generator]
				{
					return action_function_data{damage, new (std::malloc(sizeof(float))) float(data_generator())};
				}},
				choice_generator{[&data_generator]
				{
					return action_function_data{heal, new (std::malloc(sizeof(float))) float(data_generator())};
				}},
				choice_generator{[&data_generator]
				{
					return action_function_data{buff, new (std::malloc(sizeof(float))) float(data_generator())};
				}}
			};

			std::generate_n(std::back_inserter(actions), num_data, [choices, action_generator = generator(std::size_t{}, std::size(choices) - 1)]() mutable
			{
				return choices[action_generator()]();
			});
			
			start = std::chrono::steady_clock::now();
			for(auto index = std::size_t{}, size = characters.size(); index < size; ++index)
			{
				results[index] = actions[index].function(characters[index], actions[index].data);
			}
			end = std::chrono::steady_clock::now();

			for(auto& current_action : actions)
			{
				std::free(current_action.data);
			}
		}
		else if(chosen_test_type == test_type::polymorphic_objects)
		{
			std::cout << "Testing for polymorphic objects...\n";

			auto actions = std::vector<std::unique_ptr<action_class>>();

			using choice_generator = std::function<std::unique_ptr<action_class>()>;
			auto choices = std::array
			{
				choice_generator{[&data_generator]
				{
					return std::make_unique<damage_action_class>(damage_action_class{data_generator()});
				}},
				choice_generator{[&data_generator]
				{
					return std::make_unique<heal_action_class>(heal_action_class{data_generator()});
				}},
				choice_generator{[&data_generator]
				{
					return std::make_unique<buff_action_class>(buff_action_class{data_generator()});
				}}
			};

			std::generate_n(std::back_inserter(actions), num_data, [choices, action_generator = generator(std::size_t{}, std::size(choices) - 1)]() mutable
			{
				return choices[action_generator()]();
			});
			
			start = std::chrono::steady_clock::now();
			for(auto index = std::size_t{}, size = characters.size(); index < size; ++index)
			{
				results[index] = (*actions[index])(characters[index]);
			}
			end = std::chrono::steady_clock::now();
		}
		else if(chosen_test_type == test_type::std_function)
		{
			std::cout << "Testing for standard functions...\n";

			auto actions = std::vector<std_action_function>(num_data);

			using choice_generator = std::function<std_action_function()>;
			auto choices = std::array
			{
				choice_generator{[&data_generator]
				{
					return [damage = data_generator()](auto subject)
					{
						return character{subject.health - damage, subject.attack}; 
					};
				}},
				choice_generator{[&data_generator]
				{
					return [heal = data_generator()](auto subject)
					{
						return character{subject.health + heal, subject.attack}; 
					};
				}},
				choice_generator{[&data_generator]
				{
					return [buff = data_generator()](auto subject)
					{
						return character{subject.health, subject.attack + buff}; 
					};
				}}
			};

			std::generate(actions.begin(), actions.end(), [choices, action_generator = generator(std::size_t{}, std::size(choices) - 1)]() mutable
			{
				return choices[action_generator()]();
			});
			
			start = std::chrono::steady_clock::now();
			for(auto index = std::size_t{}, size = characters.size(); index < size; ++index)
			{
				results[index] = actions[index](characters[index]);
			}
			end = std::chrono::steady_clock::now();
		}
		else if(chosen_test_type == test_type::std_variant)
		{
			std::cout << "Testing for standard variant...\n";

			auto actions = std::vector<action_variant>();

			using choice_generator = std::function<action_variant()>;
			auto choices = std::array
			{
				choice_generator{[&data_generator]
				{
					return damage_variant(data_generator());
				}},
				choice_generator{[&data_generator]
				{
					return heal_variant(data_generator());
				}},
				choice_generator{[&data_generator]
				{
					return buff_variant(data_generator());
				}}
			};

			std::generate_n(std::back_inserter(actions), num_data, [choices, action_generator = generator(std::size_t{}, std::size(choices) - 1)]() mutable
			{
				return choices[action_generator()]();
			});
			
			start = std::chrono::steady_clock::now();
			for(auto index = std::size_t{}, size = characters.size(); index < size; ++index)
			{
				results[index] = std::visit([subject = characters[index]](auto chosen_action)
				{
					return chosen_action(subject);
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
};