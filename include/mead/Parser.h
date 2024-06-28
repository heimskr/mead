#pragma once

#include "mead/ASTNode.h"
#include "mead/Token.h"
#include "mead/TypeDB.h"

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

	class Parser {
		private:
			std::vector<ASTNodePtr> astNodes;
			TypeDB typeDB;

		public:
			Parser();

			/** Returns the token where parsing failed if applicable, or nothing otherwise. */
			std::optional<Token> parse(std::span<const Token> tokens);

			const auto & getNodes() const { return astNodes; }

		private:
			ASTNodePtr add(ASTNodePtr);
			const Token * peek(std::span<const Token> tokens, TokenType token_type);
			const Token * take(std::span<const Token> &tokens, TokenType token_type);
			ParseResult takeFunctionPrototype(std::span<const Token> &tokens);
			ParseResult takeFunctionDeclaration(std::span<const Token> &tokens);
			ParseResult takeFunctionDefinition(std::span<const Token> &tokens);
			ParseResult takeIdentifier(std::span<const Token> &tokens);
			ParseResult takeNumber(std::span<const Token> &tokens);
			ParseResult takeString(std::span<const Token> &tokens);
			ParseResult takeIdentifierExpression(std::span<const Token> &tokens);
			ParseResult takeNumberExpression(std::span<const Token> &tokens);
			ParseResult takeStringExpression(std::span<const Token> &tokens);
			ParseResult takeTypedVariable(std::span<const Token> &tokens);
			ParseResult takeBlock(std::span<const Token> &tokens);
			ParseResult takeStatement(std::span<const Token> &tokens);
			ParseResult takeType(std::span<const Token> &tokens, QualifiedType *);
			ParseResult takeStar(std::span<const Token> &tokens);
			ParseResult takeAmpersand(std::span<const Token> &tokens);
			ParseResult takeVariableDeclaration(std::span<const Token> &tokens);
			ParseResult takeExpression(std::span<const Token> &tokens);
			ParseResult takePrime(std::span<const Token> &tokens);
			ParseResult takeExpressionList(std::span<const Token> &tokens);
			ParseResult takeArgumentList(std::span<const Token> &tokens);
			ParseResult takeConstructorExpression(std::span<const Token> &tokens);
			ParseResult takePrefixExpression(std::span<const Token> &tokens);
			ParseResult takeUnaryPrefixExpression(std::span<const Token> &tokens);
			ParseResult takeCastExpression(std::span<const Token> &tokens);
			ParseResult takeScopePrime(std::span<const Token> &tokens);
			ParseResult takePostfixPrime(std::span<const Token> &tokens);
			ParseResult takeArgumentsPrime(std::span<const Token> &tokens);
			ParseResult takeSubscriptPrime(std::span<const Token> &tokens);
			ParseResult takeSizeExpression(std::span<const Token> &tokens);
			ParseResult takeNewExpression(std::span<const Token> &tokens);

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
	};
}
