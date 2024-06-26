#pragma once

#include "mead/ASTNode.h"
#include "mead/Token.h"
#include "mead/TypeDB.h"

#include <memory>
#include <optional>
#include <print>
#include <span>
#include <string>
#include <vector>

namespace mead {
	class ASTNode;
	class QualifiedType;

	class Parser {
		private:
			std::vector<ASTNodePtr> astNodes;
			TypeDB typeDB;

		public:
			Parser();

			/** Returns the token where parsing failed if applicable, or nothing otherwise. */
			std::optional<Token> parse(std::span<const Token> tokens);

		private:
			ASTNodePtr add(ASTNodePtr);
			const Token * peek(std::span<const Token> tokens, TokenType token_type);
			const Token * take(std::span<const Token> &tokens, TokenType token_type);
			ASTNodePtr takeFunctionPrototype(std::span<const Token> &tokens);
			ASTNodePtr takeFunctionDeclaration(std::span<const Token> &tokens);
			ASTNodePtr takeFunctionDefinition(std::span<const Token> &tokens);
			ASTNodePtr takeIdentifier(std::span<const Token> &tokens);
			ASTNodePtr takeTypedVariable(std::span<const Token> &tokens);
			ASTNodePtr takeBlock(std::span<const Token> &tokens);
			ASTNodePtr takeStatement(std::span<const Token> &tokens);
			ASTNodePtr takeType(std::span<const Token> &tokens, QualifiedType *);
			ASTNodePtr takeStar(std::span<const Token> &tokens);
			ASTNodePtr takeAmpersand(std::span<const Token> &tokens);
			std::optional<std::string> takeIdentifierPure(std::span<const Token> &tokens);
			ASTNodePtr takeVariableDeclaration(std::span<const Token> &tokens);
			ASTNodePtr takeExpression(std::span<const Token> &tokens);
			ASTNodePtr takePrime(std::span<const Token> &tokens);
			ASTNodePtr takeExpressionList(std::span<const Token> &tokens);
			ASTNodePtr takeArgumentList(std::span<const Token> &tokens);
			ASTNodePtr takeConstructorExpression(std::span<const Token> &tokens);
			ASTNodePtr takePrefixExpression(std::span<const Token> &tokens);
			ASTNodePtr takeUnaryPrefixExpression(std::span<const Token> &tokens);
			ASTNodePtr takeCastExpression(std::span<const Token> &tokens);
			ASTNodePtr takePostfixPrime(std::span<const Token> &tokens);
			ASTNodePtr takeSizeExpression(std::span<const Token> &tokens);
			ASTNodePtr takeNewExpression(std::span<const Token> &tokens);

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
			};

			Logger logger(std::string prefix) {
				Logger out{*this, std::move(prefix)};
				out("Start");
				return out;
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
