#include <bits/stdc++.h>
#include <numeric>
#include <execution>

using size = unsigned int;
using score = unsigned short;

using namespace std::literals::chrono_literals;

struct library {
	size cbooks, signup, scan_rate, id, scanned = 0, score;
	bool did_sign = false;
	std::vector<size> books;
};

struct result {
	size_t score, libs;
};

double temperature(double r) {
	return std::log(r);
}

double prob(size_t e, size_t e_new, double temp) {
	return std::exp(-double(e_new - e)/temp);
}

result calc_energy(size_t days, const std::vector<score> &scores, std::vector<library> &libraries) {
	size_t score = 0, lib = 0;
	for (size_t day = 0; day < days && lib < libraries.size(); ++day, ++lib) {
		day += libraries[lib].signup;
		libraries[lib].did_sign = true;
	}

	for (size_t day = 0; day < days; ++day) {
		for (library &l : libraries) {
			size_t last = l.scanned;
			if (l.did_sign) {
				for (size i = last; (l.scanned - last) <= l.scan_rate
					     && i < l.books.size(); ++i) {
					score += scores[l.books[i]];
					++l.scanned;
				}
			}
		}
	}
	return {score, lib};
}


int main(int, char **argv) {
	size books, clibraries, days;
	std::cin >> books >> clibraries >> days;
	std::vector<score> scores(books);
	for (score &s : scores) {
		std::cin >> s;
	}

	std::vector<library> libraries(clibraries);

	for (size i = 0; i < clibraries; ++i) {
		library &l = libraries[i];
		l.id = i;
		std::cin >> l.cbooks >> l.signup >> l.scan_rate;
		l.books.resize(l.cbooks);
		for (size &b : l.books) {
			std::cin >> b;
		}
		std::sort(std::execution::par_unseq, l.books.begin(), l.books.end(),
		          [&scores](const size &bl, const size &br) { return scores[bl] > scores[br]; });;
		l.score = std::transform_reduce(std::execution::par_unseq, l.books.begin(), l.books.end(), 0,
		                                std::plus<>(), [&scores](const size &s){ return scores[s]; });
	}

	std::sort(std::execution::par_unseq, libraries.begin(), libraries.end(),
	          // [](const library &ll, const library &lr) { return ll.signup < lr.signup; }
	          [](const library &ll, const library &lr) {
		          return (double(ll.scan_rate * ll.score) / double(ll.signup)) >
		                  (double(lr.scan_rate * lr.score) / double(lr.signup));
	          }
	          );

	auto time_max = std::chrono::seconds(atoi(argv[1]));

	result energy = calc_energy(days, scores, libraries);

	auto s = std::chrono::system_clock::now();

	std::random_device rd;
	std::mt19937 gen{rd()};
	std::uniform_real_distribution<> real_dist(0.0, 1.0);
	std::vector<size> inds(libraries.size());
	size_t kind = 0, limit = 10, last_lib;

	while ((std::chrono::system_clock::now() - s) < time_max) {
		auto now = std::chrono::system_clock::now();
		if (kind != limit) {
			for (size_t i = 0; i < libraries.size(); ++i) {
				library &l = libraries[i];
				std::uniform_int_distribution<> int_dist(0, l.cbooks - 1);
				size_t ind = int_dist(gen);
				inds[i] = ind;
				std::swap(l.books[ind], l.books[0]);
			}
		} else {
			std::uniform_int_distribution<> int_dist(0, libraries.size() - 1);
			last_lib = int_dist(gen);
			std::swap(libraries[last_lib], libraries[0]);
		}

		result energy_new = calc_energy(days, scores, libraries);
		double temp = temperature(double(time_max.count()) / double((now - s).count()));
		if (prob(energy.score, energy_new.score, temp) >= real_dist(gen)) {
			energy = energy_new;
			continue;
		}

		// Undo
		if (kind != limit) {
			for (size_t i = 0; i < libraries.size(); ++i) {
				library &l = libraries[i];
				std::swap(l.books[inds[i]], l.books[0]);
				l.scanned = 0;
				l.did_sign = false;
			}
			++kind;
		} else {
			std::swap(libraries[last_lib], libraries[0]);
			kind = 0;
		}
	}

	size_t index = libraries.size();
	for (size_t i = 0; i < energy.libs; ++i) {
		if (libraries[i].scanned == 0) {
			std::swap(libraries[i], libraries[index--]);
		}
	}

	std::cout << energy.libs << '\n';
	for (size_t i = 0; i < energy.libs; ++i) {
		std::cout << libraries[i].id << ' ' << libraries[i].scanned << '\n';
		for (size_t b = 0; b < libraries[i].scanned; ++b) {
			std::cout << libraries[i].books[b] << ' ';
		}
		std::cout << '\n';
	}

	std::cout << energy.score << '\n';
}
