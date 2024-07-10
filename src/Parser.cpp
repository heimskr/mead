#include "mead/Parser.h"
#include "mead/QualifiedType.h"
#include "mead/Util.h"

#include <cassert>
#include <expected>
#include <print>
#include <set>

namespace {
	mead::NodeType getUnaryNodeType(mead::TokenType token_type) {
		using mead::TokenType;
		using enum mead::NodeType;
		switch (token_type) {
			case TokenType::Plus:  return UnaryPlus;
			case TokenType::Minus: return UnaryMinus;
			case TokenType::Bang:  return LogicalNot;
			case TokenType::Tilde: return BitwiseNot;
			default: return Invalid;
		}
	}
}

namespace mead {
	Parser::Parser() = default;

	std::optional<Token> Parser::parse(std::span<const Token> tokens) {
		auto log = logger("parse");

		int item = 0;

		for (;;) {
			log("Item {}", ++item);
			if (tokens.empty()) {
				return std::nullopt;
			}

			if (ParseResult result = takeVariableDeclaration(tokens)) {
				log("Adding variable declaration @ {}", (*result)->location());
				add(*result);
			// } else if (ParseResult result = takeVariableDefinition(tokens)) {
			// 	add(*result);
			} else if (ParseResult result = takeFunctionDeclaration(tokens)) {
				log("Adding function declaration @ {}", (*result)->location());
				add(*result);
			} else if (ParseResult result = takeFunctionDefinition(tokens)) {
				log("Adding function definition @ {}", (*result)->location());
				add(*result);
			} else if (const Token *semicolon = take(tokens, TokenType::Semicolon)) {
				log("Skipping semicolon @ {}", semicolon->location);
				;
			} else {
				log("Giving up at {}", tokens.front().location);
				return tokens.front();
			}
		}
	}

	ASTNodePtr Parser::add(ASTNodePtr node) {
		astNodes.push_back(node);
		return node;
	}

	const Token * Parser::peek(std::span<const Token> tokens, TokenType token_type) {
		if (tokens.empty()) {
			return nullptr;
		}

		if (tokens.front().type != token_type) {
			return nullptr;
		}

		return &tokens.front();
	}

	const Token * Parser::peek(std::span<const Token> tokens) {
		return tokens.empty()? nullptr : &tokens.front();
	}

	const Token * Parser::take(std::span<const Token> &tokens, TokenType token_type) {
		if (!peek(tokens, token_type)) {
			return nullptr;
		}

		const Token *token = &tokens.front();
		tokens = tokens.subspan(1);
		return token;
	}

	ParseResult Parser::takeFunctionPrototype(std::span<const Token> &tokens) {
		auto log = logger("takeFunctionPrototype");

		if (tokens.empty()) {
			log("No tokens");
			return nullptr;
		}

		Saver saver{tokens};

		ASTNodePtr node = ASTNode::make(NodeType::FunctionPrototype, tokens.front());

		if (!take(tokens, TokenType::Fn)) {
			return log.fail("No 'fn'", tokens);
		}

		ParseResult name = takeIdentifier(tokens);

		if (!name) {
			return log.fail("No function name", tokens, name);
		}

		if (!take(tokens, TokenType::OpeningParen)) {
			return log.fail("No '('", tokens);
		}

		(*name)->reparent(node);

		std::vector<ASTNodePtr> variables;

		if (!take(tokens, TokenType::ClosingParen)) {
			do {
				ParseResult variable = takeTypedVariable(tokens);

				if (!variable) {
					return log.fail("No variable", tokens, variable);
				}

				variables.push_back(std::move(variable.value()));
			} while (take(tokens, TokenType::Comma));

			if (!take(tokens, TokenType::ClosingParen)) {
				log("No ')' @ {}", tokens.front());
				return nullptr;
			}
		}

		if (take(tokens, TokenType::Arrow)) {
			if (ParseResult return_type = takeType(tokens, true, nullptr)) {
				(*return_type)->reparent(node);
			} else {
				return log.fail("No return type", tokens, return_type);
			}
		} else {
			node->add(NodeType::Type, Token(TokenType::Void, "void", {}));
		}

		for (const ASTNodePtr &variable : variables) {
			variable->reparent(node);
		}

		saver.cancel();
		return log.success(node);
	}

	ParseResult Parser::takeFunctionDeclaration(std::span<const Token> &tokens) {
		auto log = logger("takeFunctionDeclaration");
		Saver saver{tokens};

		ParseResult prototype = takeFunctionPrototype(tokens);

		if (!prototype) {
			return log.fail("No prototype", tokens, prototype);
		}

		if (!take(tokens, TokenType::Semicolon)) {
			return log.fail("No ';'", tokens);
		}

		ASTNodePtr node = ASTNode::make(NodeType::FunctionDeclaration, (*prototype)->token);
		(*prototype)->reparent(node);

		saver.cancel();
		return prototype;
	}

	ParseResult Parser::takeFunctionDefinition(std::span<const Token> &tokens) {
		auto log = logger("takeFunctionDefinition");
		Saver saver{tokens};

		ParseResult prototype = takeFunctionPrototype(tokens);

		if (!prototype) {
			return log.fail("No prototype", tokens, prototype);
		}

		ParseResult block = takeBlock(tokens);

		if (!block) {
			return log.fail("No block", tokens, block);
		}

		ASTNodePtr node = ASTNode::make(NodeType::FunctionDefinition, (*prototype)->token);
		(*prototype)->reparent(node);
		(*block)->reparent(node);

		saver.cancel();
		return log.success(node);
	}

	ParseResult Parser::takeIdentifier(std::span<const Token> &tokens) {
		auto log = logger("takeIdentifier");

		if (const Token *identifier = take(tokens, TokenType::Identifier)) {
			return log.success(ASTNode::make(NodeType::Identifier, *identifier));
		}

		return log.fail("No identifier", tokens);
	}

	ParseResult Parser::takeNumber(std::span<const Token> &tokens) {
		auto log = logger("takeNumber");

		const Token *number = take(tokens, TokenType::IntegerLiteral);

		if (!number) {
			number = take(tokens, TokenType::FloatingLiteral);
			if (!number) {
				return log.fail("No number", tokens);
			}
		}

		return log.success(ASTNode::make(NodeType::Number, *number));
	}

	ParseResult Parser::takeString(std::span<const Token> &tokens) {
		auto log = logger("takeString");

		if (const Token *string = take(tokens, TokenType::StringLiteral)) {
			return log.success(ASTNode::make(NodeType::String, *string));
		}

		return log.fail("No string", tokens);
	}

	ParseResult Parser::takeParenthetical(std::span<const Token> &tokens) {
		auto log = logger("takeParenthetical");

		if (tokens.empty()) {
			return log.fail("No tokens", tokens);
		}

		Saver saver{tokens};

		if (!take(tokens, TokenType::OpeningParen)) {
			return log.fail("No '('", tokens);
		}

		ParseResult expr = takeExpression1(tokens);

		if (!expr) {
			return log.fail("No expression", tokens, expr);
		}

		if (!take(tokens, TokenType::ClosingParen)) {
			return log.fail("No ')'", tokens);
		}

		saver.cancel();
		return log.success(expr);
	}

	ParseResult Parser::takeTypedVariable(std::span<const Token> &tokens) {
		auto log = logger("takeTypedVariable");

		if (tokens.empty()) {
			return log.fail("No tokens", tokens);
		}

		Saver saver{tokens};

		ParseResult name = takeIdentifier(tokens);

		if (!name) {
			return log.fail("No name", tokens, name);
		}

		if (!take(tokens, TokenType::Colon)) {
			return log.fail("No ':'", tokens);
		}

		ParseResult type = takeType(tokens, true, nullptr);

		if (!type) {
			return log.fail("No type", tokens, type);
		}

		ASTNodePtr node = ASTNode::make(NodeType::VariableDeclaration, saver->front());
		(*name)->reparent(node);
		(*type)->reparent(node);

		saver.cancel();
		return log.success(node);
	}

	ParseResult Parser::takeBlock(std::span<const Token> &tokens) {
		auto log = logger("takeBlock");
		Saver saver{tokens};

		if (!take(tokens, TokenType::OpeningBrace)) {
			return log.fail("No '{'", tokens);
		}

		ASTNodePtr node = ASTNode::make(NodeType::Block, saver->front());

		while (!take(tokens, TokenType::ClosingBrace)) {
			ParseResult statement = takeStatement(tokens);

			if (!statement) {
				return log.fail("No statement", tokens, statement);
			}

			(*statement)->reparent(node);
		}

		saver.cancel();
		return log.success(node);
	}

	ParseResult Parser::takeStatement(std::span<const Token> &tokens) {
		auto log = logger("takeStatement");

		if (ParseResult node = takeVariableDeclaration(tokens)) {
			return log.success(node);
		}

		if (ParseResult node = takeBlock(tokens)) {
			return log.success(node);
		}

		if (ParseResult node = takeExpression1(tokens)) {
			if (!take(tokens, TokenType::Semicolon)) {
				return log.fail("Expression statement is missing a semicolon", tokens);
			}

			return log.success(node);
		}

		if (const Token *semicolon = take(tokens, TokenType::Semicolon)) {
			return ASTNode::make(NodeType::EmptyStatement, *semicolon);
		}

		return log.fail("No statement", tokens);
	}

	ParseResult Parser::takeType(std::span<const Token> &tokens, bool include_qualifiers, QualifiedType *type_out) {
		auto log = logger("takeType");

		if (tokens.empty()) {
			return log.fail("No tokens", tokens);
		}

		Saver saver{tokens};

		std::vector<ASTNodePtr> pieces;

		std::optional<NamespacedName> name;

		ASTNodePtr node;

		if (const Token *int_type = take(tokens, TokenType::IntegerType)) {
			node = ASTNode::make(NodeType::Type, *int_type);
		} else {
			do {
				if (ParseResult piece = takeIdentifier(tokens)) {
					pieces.push_back(std::move(*piece));
				} else {
					return log.fail("No identifier", tokens);
				}
			} while (take(tokens, TokenType::DoubleColon));

			assert(!pieces.empty());

			std::vector<std::string> namespaces;

			for (size_t i = 0; i < pieces.size() - 1; ++i) {
				namespaces.push_back(pieces[i]->token.value);
			}

			name.emplace(std::move(namespaces), pieces.back()->token.value);
			node = ASTNode::make(NodeType::Type, saver->at((pieces.size() - 1) * 2));
		}

		std::vector<bool> pointer_consts;
		bool is_const = false;
		bool is_reference = false;
		bool ref_found = false;

		if (include_qualifiers) {
			for (;;) {
				if (const Token *token = take(tokens, TokenType::Const)) {
					node->add(NodeType::Const, *token);
					if (const Token *star = take(tokens, TokenType::Star)) {
						node->add(NodeType::Pointer, *star);
						pointer_consts.push_back(true);
					} else if (const Token *ampersand = take(tokens, TokenType::Ampersand)) {
						node->add(NodeType::Reference, *ampersand);
						is_const = true;
						is_reference = true;
						ref_found = true;
					}
				} else if (const Token *star = take(tokens, TokenType::Star)) {
					node->add(NodeType::Pointer, *star);
					pointer_consts.push_back(false);
				} else if (const Token *ampersand = take(tokens, TokenType::Ampersand)) {
					if (ref_found) {
						return log.fail("Ref already found", tokens);
					}

					node->add(NodeType::Reference, *ampersand);
					is_const = false;
					is_reference = true;
					ref_found = true;
				} else {
					break;
				}
			}
		}

		if (type_out) {
			if (!name) {
				name.emplace(std::vector<std::string>{}, node->token.value);
			}

			TypePtr type = Type::make(std::move(name.value()));
			typeDB.insert(type);
			*type_out = QualifiedType(std::move(pointer_consts), is_const, is_reference, std::move(type));
		}

		for (const ASTNodePtr &piece : pieces) {
			piece->reparent(node);
		}

		saver.cancel();
		return log.success(node);
	}

	ParseResult Parser::takeVariableDeclaration(std::span<const Token> &tokens) {
		auto log = logger("takeVariableDeclaration");
		Saver saver{tokens};

		ParseResult node = takeTypedVariable(tokens);

		if (!node) {
			return log.fail("No typed variable", tokens, node);
		}

		if (!take(tokens, TokenType::Semicolon)) {
			return log.fail("No ';'", tokens);
		}

		saver.cancel();
		return log.success(node);
	}

	ParseResult Parser::takeExpression1(std::span<const Token> &tokens) {
		auto log = logger("takeExpression1");

		Saver saver{tokens};

		// E2 E1'
		if (ParseResult deeper = takeExpression2(tokens)) {
			return log.success(takePrime1(tokens, *deeper), saver);
		}

		return log.fail("E1 failed", tokens);
	}

	ParseResult Parser::takePrime1(std::span<const Token> &tokens, const ASTNodePtr &lhs) {
		auto log = logger("takePrime1");

		Saver saver{tokens};

		// "::" ident E1'
		if (const Token *scope_token = take(tokens, TokenType::DoubleColon)) {
			if (ParseResult ident = takeIdentifier(tokens)) {
				ASTNodePtr node = ASTNode::make(NodeType::Scope, *scope_token);
				lhs->reparent(node);

				ParseResult prime1 = takePrime1(tokens, node);
				saver.cancel();
				return log.success(prime1);
			}
		}

		saver.restore();

		return log.success(lhs, saver);
	}

	ParseResult Parser::takeExpression2(std::span<const Token> &tokens) {
		auto log = logger("takeExpression2");

		Saver saver{tokens};

		// E3 E2'
		if (ParseResult deeper = takeExpression3(tokens)) {
			return log.success(takePrime2(tokens, *deeper), saver);
		}

		// Type "(" Exprs ")" E2'
		if (ParseResult type = takeType(tokens, false, nullptr)) {
			if (const Token *opening = take(tokens, TokenType::OpeningParen)) {
				if (ParseResult args = takeExpressionList(tokens)) {
					if (take(tokens, TokenType::ClosingParen)) {
						ASTNodePtr node = ASTNode::make(NodeType::ConstructorCall, *opening);
						(*type)->reparent(node);
						(*args)->reparent(node);
						return log.success(takePrime2(tokens, node), saver);
					}
				}
			}
		}

		return log.fail("E2 failed", tokens);
	}

	ParseResult Parser::takePrime2(std::span<const Token> &tokens, const ASTNodePtr &lhs) {
		auto log = logger("takePrime2");

		Saver saver{tokens};

		// ("++" | "--") E2'
		for (TokenType token_type : {TokenType::DoublePlus, TokenType::DoubleMinus}) {
			if (const Token *token = take(tokens, token_type)) {
				NodeType node_type = token_type == TokenType::DoublePlus? NodeType::PostfixIncrement : NodeType::PostfixDecrement;
				ASTNodePtr node = ASTNode::make(node_type, *token);
				lhs->reparent(node);
				return log.success(takePrime2(tokens, node), saver);
			}
		}

		// "(" Exprs ")" E2'
		if (const Token *opening = take(tokens, TokenType::OpeningParen)) {
			if (ParseResult args = takeExpressionList(tokens)) {
				if (take(tokens, TokenType::ClosingParen)) {
					ASTNodePtr node = ASTNode::make(NodeType::FunctionCall, *opening);
					lhs->reparent(node);
					(*args)->reparent(node);
					return log.success(takePrime2(tokens, node), saver);
				}
			}

			return log.fail("Invalid function call", tokens);
		}

		// "[" E1 "]" E2'
		if (const Token *opening = take(tokens, TokenType::OpeningSquare)) {
			if (ParseResult subscript = takeExpression1(tokens)) {
				if (take(tokens, TokenType::ClosingParen)) {
					ASTNodePtr node = ASTNode::make(NodeType::Subscript, *opening);
					lhs->reparent(node);
					(*subscript)->reparent(node);
					return log.success(takePrime2(tokens, node), saver);
				}
			}

			return log.fail("Invalid subscript", tokens);
		}

		// "." (ident | "*" | "&") E2'
		if (const Token *dot = take(tokens, TokenType::Dot)) {
			if (ParseResult ident = takeIdentifier(tokens)) {
				ASTNodePtr node = ASTNode::make(NodeType::AccessMember, *dot);
				lhs->reparent(node);
				(*ident)->reparent(node);
				return log.success(takePrime2(tokens, node), saver);
			}

			if (const Token *star = take(tokens, TokenType::Star)) {
				ASTNodePtr node = ASTNode::make(NodeType::Deref, *star);
				lhs->reparent(node);
				return log.success(takePrime2(tokens, node), saver);
			}

			if (const Token *ampersand = take(tokens, TokenType::Ampersand)) {
				ASTNodePtr node = ASTNode::make(NodeType::GetAddress, *ampersand);
				lhs->reparent(node);
				return log.success(takePrime2(tokens, node), saver);
			}

			return log.fail("Invalid dot expression", tokens);
		}

		return log.success(lhs, saver);
	}

	ParseResult Parser::takeExpression3(std::span<const Token> &tokens) {
		using enum TokenType;

		auto log = logger("takeExpression3");

		Saver saver{tokens};

		// ("++" | "--") E3
		for (TokenType token_type : {DoublePlus, DoubleMinus}) {
			if (const Token *token = take(tokens, token_type)) {
				if (ParseResult rhs = takeExpression3(tokens)) {
					NodeType node_type = token_type == DoublePlus? NodeType::PrefixIncrement : NodeType::PrefixDecrement;
					ASTNodePtr node = ASTNode::make(node_type, *token);
					(*rhs)->reparent(node);
					return log.success(node, saver);
				} else {
					return log.fail("No E3 in prefix expression", tokens, rhs);
				}
			}
		}

		// ("+" | "-" | "!" | "~") E3
		for (TokenType token_type : {Plus, Minus, Bang, Tilde}) {
			if (const Token *token = take(tokens, token_type)) {
				if (ParseResult rhs = takeExpression3(tokens)) {
					ASTNodePtr node = ASTNode::make(getUnaryNodeType(token_type), *token);
					(*rhs)->reparent(node);
					return log.success(node, saver);
				} else {
					// TODO: interferes with binary +/-?
					return log.fail("Invalid E3 in unary", tokens, rhs);
				}
			}
		}

		// cast "<" Type ">" "(" E1 ")"
		if (const Token *cast = take(tokens, Cast)) {
			if (take(tokens, OpeningAngle)) {
				if (ParseResult type = takeType(tokens, true, nullptr)) {
					if (take(tokens, ClosingAngle)) {
						if (ParseResult subexpr = takeParenthetical(tokens)) {
							ASTNodePtr node = ASTNode::make(NodeType::Cast, *cast);
							(*type)->reparent(node);
							(*subexpr)->reparent(node);
							return log.success(node, saver);
						}
					}
				}
			}

			return log.fail("Invalid cast", tokens);
		}

		// "sizeof" "(" E1 ")"
		if (const Token *sizeof_token = take(tokens, Sizeof)) {
			if (ParseResult subexpr = takeParenthetical(tokens)) {
				ASTNodePtr node = ASTNode::make(NodeType::Sizeof, *sizeof_token);
				(*subexpr)->reparent(node);
				return log.success(node, saver);
			} else {
				return log.fail("Invalid sizeof", tokens, subexpr);
			}
		}

		// "new" Type ("(" Exprs ")" | "[" E1 "]")?
		if (const Token *new_token = take(tokens, New)) {
			if (ParseResult type = takeType(tokens, false, nullptr)) {
				Saver subsaver{tokens};

				if (take(tokens, OpeningParen)) {
					if (ParseResult exprs = takeExpressionList(tokens)) {
						if (take(tokens, ClosingParen)) {
							ASTNodePtr node = ASTNode::make(NodeType::SingleNew, *new_token);
							(*type)->reparent(node);
							(*exprs)->reparent(node);
							subsaver.cancel();
							return log.success(node, saver);
						}
					}

					subsaver.restore();
				}

				if (take(tokens, OpeningSquare)) {
					if (ParseResult count = takeExpression1(tokens)) {
						if (take(tokens, ClosingSquare)) {
							ASTNodePtr node = ASTNode::make(NodeType::ArrayNew, *new_token);
							(*type)->reparent(node);
							(*count)->reparent(node);
							subsaver.cancel();
							return log.success(node, saver);
						}
					}
				}

				ASTNodePtr node = ASTNode::make(NodeType::SingleNew, *new_token);
				(*type)->reparent(node);
				return log.success(node, saver);
			}

			return log.fail("Invalid new", tokens);
		}

		// "delete" E3
		if (const Token *delete_token = take(tokens, Delete)) {
			if (ParseResult subexpr = takeExpression3(tokens)) {
				ASTNodePtr node = ASTNode::make(NodeType::Delete, *delete_token);
				(*subexpr)->reparent(node);
				return log.success(node, saver);
			} else {
				return log.fail("Invalid delete", tokens, subexpr);
			}
		}

		// E4
		if (ParseResult deeper = takeExpression4(tokens)) {
			return log.success(deeper, saver);
		}

		return log.fail("E3 failed", tokens);
	}

	ParseResult Parser::takeExpression4(std::span<const Token> &tokens) {
		auto log = logger("takeExpression4");

		Saver saver{tokens};

		if (ParseResult deeper = takeExpression5(tokens)) {
			return log.success(takePrime4(tokens, *deeper), saver);
		}



		return log.fail("E4 failed", tokens);
	}

	ParseResult Parser::takePrime4(std::span<const Token> &tokens, const ASTNodePtr &lhs) {
		auto log = logger("takePrime4");

		Saver saver{tokens};



		return log.success(lhs);
	}

	ParseResult Parser::takeExpression5(std::span<const Token> &tokens) {
		auto log = logger("takeExpression5");

		Saver saver{tokens};

		if (ParseResult deeper = takeExpression6(tokens)) {
			return log.success(takePrime5(tokens, *deeper), saver);
		}



		return log.fail("E5 failed", tokens);
	}

	ParseResult Parser::takePrime5(std::span<const Token> &tokens, const ASTNodePtr &lhs) {
		auto log = logger("takePrime5");

		Saver saver{tokens};



		return log.success(lhs);
	}

	ParseResult Parser::takeExpression6(std::span<const Token> &tokens) {
		auto log = logger("takeExpression6");

		Saver saver{tokens};

		if (ParseResult deeper = takeExpression7(tokens)) {
			return log.success(takePrime6(tokens, *deeper), saver);
		}



		return log.fail("E6 failed", tokens);
	}

	ParseResult Parser::takePrime6(std::span<const Token> &tokens, const ASTNodePtr &lhs) {
		auto log = logger("takePrime6");

		Saver saver{tokens};



		return log.success(lhs);
	}

	ParseResult Parser::takeExpression7(std::span<const Token> &tokens) {
		auto log = logger("takeExpression7");

		Saver saver{tokens};

		if (ParseResult deeper = takeExpression8(tokens)) {
			return log.success(takePrime7(tokens, *deeper), saver);
		}



		return log.fail("E7 failed", tokens);
	}

	ParseResult Parser::takePrime7(std::span<const Token> &tokens, const ASTNodePtr &lhs) {
		auto log = logger("takePrime7");

		Saver saver{tokens};



		return log.success(lhs);
	}

	ParseResult Parser::takeExpression8(std::span<const Token> &tokens) {
		auto log = logger("takeExpression8");

		Saver saver{tokens};

		if (ParseResult deeper = takeExpression9(tokens)) {
			return log.success(takePrime8(tokens, *deeper), saver);
		}



		return log.fail("E8 failed", tokens);
	}

	ParseResult Parser::takePrime8(std::span<const Token> &tokens, const ASTNodePtr &lhs) {
		auto log = logger("takePrime8");

		Saver saver{tokens};



		return log.success(lhs);
	}

	ParseResult Parser::takeExpression9(std::span<const Token> &tokens) {
		auto log = logger("takeExpression9");

		Saver saver{tokens};

		if (ParseResult deeper = takeExpression10(tokens)) {
			return log.success(takePrime9(tokens, *deeper), saver);
		}



		return log.fail("E9 failed", tokens);
	}

	ParseResult Parser::takePrime9(std::span<const Token> &tokens, const ASTNodePtr &lhs) {
		auto log = logger("takePrime9");

		Saver saver{tokens};



		return log.success(lhs);
	}

	ParseResult Parser::takeExpression10(std::span<const Token> &tokens) {
		auto log = logger("takeExpression10");

		Saver saver{tokens};

		if (ParseResult deeper = takeExpression11(tokens)) {
			return log.success(takePrime10(tokens, *deeper), saver);
		}



		return log.fail("E10 failed", tokens);
	}

	ParseResult Parser::takePrime10(std::span<const Token> &tokens, const ASTNodePtr &lhs) {
		auto log = logger("takePrime10");

		Saver saver{tokens};



		return log.success(lhs);
	}

	ParseResult Parser::takeExpression11(std::span<const Token> &tokens) {
		auto log = logger("takeExpression11");

		Saver saver{tokens};

		if (ParseResult deeper = takeExpression12(tokens)) {
			return log.success(takePrime11(tokens, *deeper), saver);
		}



		return log.fail("E11 failed", tokens);
	}

	ParseResult Parser::takePrime11(std::span<const Token> &tokens, const ASTNodePtr &lhs) {
		auto log = logger("takePrime11");

		Saver saver{tokens};



		return log.success(lhs);
	}

	ParseResult Parser::takeExpression12(std::span<const Token> &tokens) {
		auto log = logger("takeExpression12");

		Saver saver{tokens};

		if (ParseResult deeper = takeExpression13(tokens)) {
			return log.success(takePrime12(tokens, *deeper), saver);
		}



		return log.fail("E12 failed", tokens);
	}

	ParseResult Parser::takePrime12(std::span<const Token> &tokens, const ASTNodePtr &lhs) {
		auto log = logger("takePrime12");

		Saver saver{tokens};



		return log.success(lhs);
	}

	ParseResult Parser::takeExpression13(std::span<const Token> &tokens) {
		auto log = logger("takeExpression13");

		Saver saver{tokens};

		if (ParseResult deeper = takeExpression14(tokens)) {
			return log.success(takePrime13(tokens, *deeper), saver);
		}



		return log.fail("E13 failed", tokens);
	}

	ParseResult Parser::takePrime13(std::span<const Token> &tokens, const ASTNodePtr &lhs) {
		auto log = logger("takePrime13");

		Saver saver{tokens};



		return log.success(lhs);
	}

	ParseResult Parser::takeExpression14(std::span<const Token> &tokens) {
		auto log = logger("takeExpression14");

		Saver saver{tokens};

		if (ParseResult deeper = takeExpression15(tokens)) {
			return log.success(takePrime14(tokens, *deeper), saver);
		}



		return log.fail("E14 failed", tokens);
	}

	ParseResult Parser::takePrime14(std::span<const Token> &tokens, const ASTNodePtr &lhs) {
		auto log = logger("takePrime14");

		Saver saver{tokens};



		return log.success(lhs);
	}

	ParseResult Parser::takeExpression15(std::span<const Token> &tokens) {
		auto log = logger("takeExpression15");

		Saver saver{tokens};

		if (ParseResult deeper = takeExpression16(tokens)) {
			return log.success(deeper, saver);
		}

		return log.fail("E15 failed", tokens);
	}

	ParseResult Parser::takeExpression16(std::span<const Token> &tokens) {
		auto log = logger("takeExpression16");

		Saver saver{tokens};

		if (ParseResult deeper = takeExpression17(tokens)) {
			return log.success(takePrime16(tokens, *deeper), saver);
		}



		return log.fail("E16 failed", tokens);
	}

	ParseResult Parser::takePrime16(std::span<const Token> &tokens, const ASTNodePtr &lhs) {
		auto log = logger("takePrime16");

		Saver saver{tokens};



		return log.success(lhs);
	}

	ParseResult Parser::takeExpression17(std::span<const Token> &tokens) {
		auto log = logger("takeExpression17");

		Saver saver{tokens};



		return log.fail("E17 failed", tokens);
	}


















	ParseResult Parser::takeExpressionList(std::span<const Token> &tokens) {
		return logger("takeExpressionList").fail("Not implemented", tokens);
	}

	ParseResult Parser::takeConstructorExpression(std::span<const Token> &tokens) {
		auto log = logger("takeConstructorExpression");

		if (tokens.empty()) {
			return log.fail("No tokens", tokens);
		}

		const Token &anchor = tokens.front();
		Saver saver{tokens};

		ParseResult type = takeType(tokens, nullptr);

		if (!type) {
			return log.fail("No type", tokens, type);
		}

		if (!take(tokens, TokenType::OpeningParen)) {
			return log.fail("No '('", tokens);
		}

		ParseResult list = takeExpressionList(tokens);

		if (!list) {
			return log.fail("No list", tokens, list);
		}

		if (!take(tokens, TokenType::ClosingParen)) {
			return log.fail("No ')'", tokens);
		}

		ASTNodePtr node = ASTNode::make(NodeType::ConstructorExpression, anchor);
		(*type)->reparent(node);
		(*list)->reparent(node);

		if (with_prime) {
			ParseResult prime = takePrime(tokens, node);
			saver.cancel();
			return log.success(prime);
		}

		saver.cancel();
		return log.success(node);
	}

	ParseResult Parser::takePrefixExpression(std::span<const Token> &tokens) {
		auto log = logger("takePrefix");
		Saver saver{tokens};

		const Token *oper = take(tokens, TokenType::DoublePlus);

		if (!oper) {
			oper = take(tokens, TokenType::DoubleMinus);
			if (!oper) {
				return log.fail("No '++' or '--'", tokens);
			}
		}

		ParseResult subexpr = withMinPrecedence(prefixPrecedence, [&] { return takeExpression(tokens, false); });

		if (!subexpr) {
			return log.fail("No subexpression", tokens, subexpr);
		}

		ASTNodePtr node = ASTNode::make(NodeType::PrefixExpression, *oper);
		(*subexpr)->reparent(node);

		if (with_prime) {
			ParseResult prime = takePrime(tokens, node);
			saver.cancel();
			return log.success(prime);
		}

		saver.cancel();
		return log.success(node);
	}

	ParseResult Parser::takeUnaryPrefixExpression(std::span<const Token> &tokens) {
		auto log = logger("takeUnaryPrefixExpression");
		Saver saver{tokens};

		const Token *oper = nullptr;

		if (const Token *plus = take(tokens, TokenType::Plus)) {
			oper = plus;
		} else if (const Token *minus = take(tokens, TokenType::Minus)) {
			oper = minus;
		} else if (const Token *bang = take(tokens, TokenType::Bang)) {
			oper = bang;
		} else if (const Token *tilde = take(tokens, TokenType::Tilde)) {
			oper = tilde;
		} else {
			return log.fail("No operator", tokens);
		}

		ParseResult expr = withMinPrecedence(prefixPrecedence, [&] { return takeExpression(tokens, true); });

		if (!expr) {
			return log.fail("No subexpression", tokens, expr);
		}

		ASTNodePtr node = ASTNode::make(NodeType::UnaryExpression, *oper);
		(*expr)->reparent(node);

		if (with_prime) {
			ParseResult prime = takePrime(tokens, node);
			saver.cancel();
			return log.success(prime);
		}

		saver.cancel();
		return log.success(node);
	}

	ParseResult Parser::takeCastExpression(std::span<const Token> &tokens) {
		auto log = logger("takeCastExpression");
		Saver saver{tokens};

		const Token *cast = take(tokens, TokenType::Cast);

		if (!cast) {
			return log.fail("No cast token", tokens);
		}

		if (!take(tokens, TokenType::OpeningAngle)) {
			return log.fail("No '<'", tokens);
		}

		ParseResult type = takeType(tokens, nullptr);

		if (!type) {
			return log.fail("No type", tokens, type);
		}

		if (!take(tokens, TokenType::ClosingAngle)) {
			return log.fail("No '>'", tokens);
		}

		ParseResult expr = takeParenthetical(tokens, false);

		if (!expr) {
			return log.fail("No subexpression", tokens, expr);
		}

		ASTNodePtr lhs = ASTNode::make(NodeType::CastExpression, *cast);
		(*type)->reparent(lhs);
		(*expr)->reparent(lhs);

		if (with_prime) {
			ParseResult prime = takePrime(tokens, lhs);
			saver.cancel();
			return log.success(prime);
		}

		saver.cancel();
		return log.success(lhs);
	}

	ParseResult Parser::takeScopePrime(std::span<const Token> &tokens, const ASTNodePtr &lhs) {
		auto log = logger("takeScopePrime");
		Saver saver{tokens};

		const Token *scope = take(tokens, TokenType::DoubleColon);

		if (!scope) {
			return log.fail("No '::'", tokens);
		}

		ParseResult identifier = takeIdentifier(tokens, true);

		if (!identifier) {
			return log.fail("No identifier", tokens, identifier);
		}

		ASTNodePtr node = ASTNode::make(NodeType::Scope, *scope);
		lhs->reparent(node);
		(*identifier)->reparent(node);

		ParseResult prime = takePrime(tokens, node);

		saver.cancel();
		return log.success(prime);
	}

	ParseResult Parser::takePostfixPrime(std::span<const Token> &tokens, const ASTNodePtr &lhs) {
		auto log = logger("takePostfixPrime");
		Saver saver{tokens};

		const Token *oper = take(tokens, TokenType::DoublePlus);

		if (!oper) {
			oper = take(tokens, TokenType::DoubleMinus);
			if (!oper) {
				return log.fail("No operator", tokens);
			}
		}

		ASTNodePtr node = ASTNode::make(NodeType::Postfix, *oper);
		lhs->reparent(node);

		ParseResult prime = takePrime(tokens, node);

		saver.cancel();

		return log.success(prime);
	}

	ParseResult Parser::takeArgumentsPrime(std::span<const Token> &tokens, const ASTNodePtr &lhs) {
		auto log = logger("takeArgumentsPrime");
		Saver saver{tokens};

		const Token *opening = take(tokens, TokenType::OpeningParen);

		if (!opening) {
			return log.fail("No '('", tokens);
		}

		ParseResult arguments = takeExpressionList(tokens);

		if (!arguments) {
			return log.fail("No arguments", tokens, arguments);
		}

		if (!take(tokens, TokenType::ClosingParen)) {
			return log.fail("No ')'", tokens);
		}

		ASTNodePtr node = ASTNode::make(NodeType::Arguments, *opening);
		lhs->reparent(node);
		(*arguments)->reparent(node);

		ParseResult prime = takePrime(tokens, node);

		saver.cancel();
		return log.success(prime);
	}

	ParseResult Parser::takeSubscriptPrime(std::span<const Token> &tokens, const ASTNodePtr &lhs) {
		auto log = logger("takeSubscriptPrime");
		Saver saver{tokens};

		const Token *opening = take(tokens, TokenType::OpeningSquare);

		if (!opening) {
			return log.fail("No '['", tokens);
		}

		ParseResult subexpr = takeExpression(tokens, true);

		if (!subexpr) {
			return log.fail("No subexpression", tokens, subexpr);
		}

		if (!take(tokens, TokenType::ClosingSquare)) {
			return log.fail("No ']'", tokens);
		}

		ASTNodePtr node = ASTNode::make(NodeType::Subscript, *opening);
		lhs->reparent(node);
		(*subexpr)->reparent(node);

		ParseResult prime = takePrime(tokens, node);

		saver.cancel();
		return log.success(prime);
	}

	ParseResult Parser::takePrimary(std::span<const Token> &tokens) {
		auto log = logger("takePrimary");

		if (tokens.empty()) {
			return log.fail("No tokens", Token{});
		}

		if (ParseResult expr = takeIdentifier(tokens, false)) {
			return log.success(expr);
		}

		if (ParseResult expr = takeNumber(tokens, false)) {
			return log.success(expr);
		}

		if (ParseResult expr = takeString(tokens, false)) {
			return log.success(expr);
		}

		if (ParseResult expr = takePrefixExpression(tokens, false)) {
			return log.success(expr);
		}

		if (ParseResult expr = takeParenthetical(tokens, false)) {
			return log.success(expr);
		}

		return log.fail("No primary expression", tokens);
	}

	ParseResult Parser::takeBinaryPrime(std::span<const Token> &tokens, const ASTNodePtr &lhs) {
		auto log = logger("takeBinaryPrime");

		if (tokens.empty()) {
			return log.fail("No tokens", Token{});
		}

		Saver saver{tokens};

		ASTNodePtr node;

		if (ParseResult binary_result = takeBinary(tokens, lhs)) {
			node = *binary_result;
			std::println("\x1b[32mBinary succeeded:\n{}\x1b[39m", *node);
		} else {
			std::println("\x1b[31mBinary failed: {} {}\x1b[39m", binary_result.error().first, binary_result.error().second);
			const Token *token = &tokens.front();

			if (!precedenceMap.contains(token->type)) {
				return log.fail("Invalid token", tokens);
			}

			tokens = tokens.subspan(1);

			ParseResult rhs = withMinPrecedence(maxBinaryPrecedence + 1, [&] { return takeExpression(tokens, true); });

			if (!rhs) {
				return log.fail("No rhs", tokens, rhs);
			}

			node = ASTNode::make(NodeType::Binary, *token);
			lhs->reparent(node);
			(*rhs)->reparent(node);
		}

		// ParseResult prime = withMinPrecedence(maxBinaryPrecedence + 1, [&] { return takePrime(tokens, node, maxBinaryPrecedence + 1); }); // ???
		ParseResult prime = takePrime(tokens, node);

		saver.cancel();
		return log.success(prime);
	}

	ParseResult Parser::takeSizeExpression(std::span<const Token> &tokens) {
		auto log = logger("takeSizeExpression");
		Saver saver{tokens};

		const Token *token = take(tokens, TokenType::Size);

		if (!token) {
			return log.fail("No '#size'", tokens);
		}

		ParseResult expr = takeParenthetical(tokens, false);

		if (!expr) {
			return log.fail("No subexpression", tokens, expr);
		}

		ASTNodePtr node = ASTNode::make(NodeType::SizeExpression, *token);
		(*expr)->reparent(node);

		if (with_prime) {
			ParseResult prime = takePrime(tokens, node);
			saver.cancel();
			return log.success(prime);
		}

		saver.cancel();
		return log.success(node);
	}

	ParseResult Parser::takeNewExpression(std::span<const Token> &tokens) {
		auto log = logger("takeNewExpression");
		Saver saver{tokens};

		const Token *token = take(tokens, TokenType::New);

		if (!token) {
			return log.fail("No 'new'", tokens);
		}

		ParseResult type = takeType(tokens, nullptr);

		if (!type) {
			return log.fail("No type", tokens, type);
		}

		if (take(tokens, TokenType::OpeningSquare)) {
			log("Found '['");
			ParseResult expr = takeExpression(tokens, true);

			if (!expr) {
				return log.fail("No subexpression", tokens, expr);
			}

			if (!take(tokens, TokenType::ClosingSquare)) {
				return log.fail("No ']'", tokens);
			}

			ASTNodePtr node = ASTNode::make(NodeType::ArrayNewExpression, *token);
			(*type)->reparent(node);
			(*expr)->reparent(node);

			if (with_prime) {
				ParseResult prime = takePrime(tokens, node);
				saver.cancel();
				return log.success(prime);
			}

			saver.cancel();
			return log.success(node);
		}

		if (take(tokens, TokenType::OpeningParen)) {
			log("Found '('");
			ParseResult list = takeExpressionList(tokens);

			if (!list) {
				return log.fail("No argument list", tokens, list);
			}

			if (!take(tokens, TokenType::ClosingParen)) {
				return log.fail("No ')'", tokens);
			}

			ASTNodePtr node = ASTNode::make(NodeType::SingleNewExpression, *token);
			(*type)->reparent(node);
			(*list)->reparent(node);

			if (with_prime) {
				ParseResult prime = takePrime(tokens, node);
				saver.cancel();
				return log.success(prime);
			}

			saver.cancel();
			return log.success(node);
		}

		log("Short new expression");

		ASTNodePtr node = ASTNode::make(NodeType::SingleNewExpression, *token);
		(*type)->reparent(node);

		if (with_prime) {
			ParseResult prime = takePrime(tokens, node);
			saver.cancel();
			return log.success(prime);
		}

		saver.cancel();
		return log.success(node);
	}

	std::optional<std::string> Parser::takeIdentifierPure(std::span<const Token> &tokens) {
		if (tokens.empty() || !peek(tokens, TokenType::Identifier)) {
			return std::nullopt;
		}

		std::string out = tokens.front().value;
		tokens = tokens.subspan(1);
		return std::make_optional(std::move(out));
	}
}
