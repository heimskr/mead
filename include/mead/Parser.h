#pragma once

#include "mead/ASTNode.h"
#include "mead/Token.h"
#include "mead/TypeDB.h"
#include "mead/Util.h"

#include <cassert>
#include <expected>
#include <memory>
#include <optional>
#include <print>
#include <span>
#include <string>
#include <vector>

namespace mead {
	class QualifiedType;

	using ParseError = std::pair<std::string, Token>;
	using ParseResult = std::expected<ASTNodePtr, ParseError>;

	enum class Associativity {None, LeftToRight, RightToLeft};

	class Parser {
		private:
			std::vector<ASTNodePtr> astNodes;
			TypeDB typeDB;
			std::vector<int> minPrecedenceStack{0};

		public:
			Parser();

			/** Returns the token where parsing failed if applicable, or nothing otherwise. */
			std::optional<Token> parse(std::span<const Token> tokens);

			const auto & getNodes() const { return astNodes; }

		private:
			ASTNodePtr add(ASTNodePtr);
			const Token * peek(std::span<const Token> tokens, TokenType token_type);
			const Token * peek(std::span<const Token> tokens);
			const Token * take(std::span<const Token> &tokens, TokenType token_type);
			ParseResult takeFunctionPrototype(std::span<const Token> &tokens);
			ParseResult takeFunctionDeclaration(std::span<const Token> &tokens);
			ParseResult takeFunctionDefinition(std::span<const Token> &tokens);
			ParseResult takeIdentifierExpression(std::span<const Token> &tokens, bool with_prime);
			ParseResult takeNumberExpression(std::span<const Token> &tokens, bool with_prime);
			ParseResult takeStringExpression(std::span<const Token> &tokens, bool with_prime);
			ParseResult takeParentheticalExpression(std::span<const Token> &tokens, bool with_prime);
			ParseResult takeTypedVariable(std::span<const Token> &tokens);
			ParseResult takeBlock(std::span<const Token> &tokens);
			ParseResult takeStatement(std::span<const Token> &tokens);
			ParseResult takeType(std::span<const Token> &tokens, QualifiedType *);
			ParseResult takeStar(std::span<const Token> &tokens);
			ParseResult takeAmpersand(std::span<const Token> &tokens);
			ParseResult takeVariableDeclaration(std::span<const Token> &tokens);
			ParseResult takeExpression(std::span<const Token> &tokens, bool with_prime);
			ParseResult takePrime(std::span<const Token> &tokens, const ASTNodePtr &lhs, int binary_precedence = 0);
			ParseResult takeExpressionList(std::span<const Token> &tokens);
			ParseResult takeArgumentList(std::span<const Token> &tokens);
			ParseResult takeConstructorExpression(std::span<const Token> &tokens, bool with_prime);
			ParseResult takePrefixExpression(std::span<const Token> &tokens, bool with_prime);
			ParseResult takeUnaryPrefixExpression(std::span<const Token> &tokens, bool with_prime);
			ParseResult takeCastExpression(std::span<const Token> &tokens, bool with_prime);
			ParseResult takeScopePrime(std::span<const Token> &tokens, const ASTNodePtr &lhs);
			ParseResult takePostfixPrime(std::span<const Token> &tokens, const ASTNodePtr &lhs);
			ParseResult takeArgumentsPrime(std::span<const Token> &tokens, const ASTNodePtr &lhs);
			ParseResult takeSubscriptPrime(std::span<const Token> &tokens, const ASTNodePtr &lhs);
			ParseResult takePrimary(std::span<const Token> &tokens);
			ParseResult takeBinaryPrime(std::span<const Token> &tokens, const ASTNodePtr &lhs);
			ParseResult takeBinary(std::span<const Token> &tokens, ASTNodePtr lhs, int min_precedence = 0);
			ParseResult takeSizeExpression(std::span<const Token> &tokens, bool with_prime);
			ParseResult takeNewExpression(std::span<const Token> &tokens, bool with_prime);

			std::optional<std::string> takeIdentifierPure(std::span<const Token> &tokens);

			std::vector<std::string> logs;

			size_t logLevel = 0;

			struct Logger {
				Parser &parser;
				std::string prefix;
				int level;
				std::vector<std::string> logs;
				bool active = true;

				Logger(Parser &parser, std::string prefix):
					parser(parser), prefix(prefix + ": "), level(parser.logLevel++) {}

				Logger(const Logger &) = delete;

				Logger(Logger &&other):
				parser(other.parser), prefix(std::move(other.prefix)), level(other.level), logs(std::move(other.logs)) {
					other.active = false;
				}

				Logger & operator=(const Logger &) = delete;
				Logger & operator=(Logger &&) = delete;

				~Logger() {
					if (!active) {
						return;
					}

					parser.logLevel = level;
					std::reverse(logs.begin(), logs.end());
					parser.logs.insert(parser.logs.end(), logs.begin(), logs.end());
				}

				template <typename... Args>
				Logger & operator()(std::format_string<Args...> format, Args &&...args) {
					logs.emplace_back(std::string(level * 2, ' ') + prefix + std::format(format, std::forward<Args>(args)...));
					return *this;
				}

				auto fail(std::string message, Token token) {
					(*this)("\x1b[31m{}\x1b[39m @ {}", message, token);
					return std::unexpected(ParseError(std::move(message), std::move(token)));
				}

				auto fail(std::string message, std::span<const Token> tokens) {
					if (tokens.empty()) {
						return fail(std::move(message), Token{});
					}

					return fail(std::move(message), tokens.front());
				}

				ParseResult && fail(const std::string &message, std::span<const Token> tokens, ParseResult &error) {
					if (tokens.empty()) {
						(*this)("\x1b[31m{}\x1b[39m", message);
					} else {
						(*this)("\x1b[31m{}\x1b[39m @ {}", message, tokens.front());
					}

					return std::move(error);
				}

				ParseResult && success(ParseResult &result) {
					(*this)("\x1b[32mSuccess\x1b[39m");
					return std::move(result);
				}

				ParseResult && success(ParseResult &&result) {
					(*this)("\x1b[32mSuccess\x1b[39m");
					return std::move(result);
				}
			};

			Logger logger(std::string prefix) {
				Logger out{*this, std::move(prefix)};
				out("Start");
				return out;
			}

			static auto fail(std::string message, Token token) {
				return std::unexpected(ParseError(std::move(message), std::move(token)));
			}

			static auto fail(std::string message, std::span<const Token> span) {
				if (span.empty()) {
					return fail(std::move(message), Token{});
				}

				return fail(std::move(message), span.front());
			}


		public:
			void print() {
				auto copy = logs;
				std::reverse(copy.begin(), copy.end());
				for (const auto &item : copy) {
					std::println("{}", item);
				}
			}

			inline int minPrecedence() {
				assert(!minPrecedenceStack.empty());
				return minPrecedenceStack.back();
			}

			inline auto saveMinPrecedence(int precedence) {
				return StackGuard(minPrecedenceStack, precedence);
			}

			template <typename Fn>
			auto withMinPrecedence(int precedence, Fn &&fn) {
				auto guard = saveMinPrecedence(precedence);
				return fn();
			}

			inline bool checkPrecedence(int precedence) {
				return precedence >= minPrecedence();
			}
	};
}
