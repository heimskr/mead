#include "mead/Parser.h"
#include "mead/QualifiedType.h"
#include "mead/Util.h"

#include "mead/node/Expression.h"
#include "mead/node/FunctionCall.h"
#include "mead/node/GetAddress.h"
#include "mead/node/Identifier.h"
#include "mead/node/TypeNode.h"

#include <array>
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
			} else if (ParseResult result = takeVariableDefinition(tokens)) {
				log("Adding variable definition @ {}", (*result)->location());
				add(*result);
			} else if (ParseResult result = takeFunctionDeclaration(tokens)) {
				log("Adding function declaration @ {}", (*result)->location());
				add(*result);
			} else if (ParseResult result = takeFunctionDefinition(tokens)) {
				log("Adding function definition @ {}", (*result)->location());
				add(*result);
			} else if (const Token *semicolon = take(tokens, TokenType::Semicolon)) {
				log("Skipping semicolon @ {}", semicolon->location);
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
			auto type_node = std::make_shared<TypeNode>(Token(TokenType::Void, "void", {}));
			type_node->reparent(node);
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

		return log.success(node, saver);
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
			return log.success(std::make_shared<Identifier>(*identifier));
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

		ParseResult expr = takeExpression16(tokens);

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
		Saver token_saver{tokens};
		Saver comma_saver{commaAllowed};
		commaAllowed = true;

		if (!take(tokens, TokenType::OpeningBrace)) {
			return log.fail("No '{'", tokens);
		}

		ASTNodePtr node = ASTNode::make(NodeType::Block, token_saver->front());

		while (!take(tokens, TokenType::ClosingBrace)) {
			ParseResult statement = takeStatement(tokens);

			if (!statement) {
				return log.fail("No statement", tokens, statement);
			}

			(*statement)->reparent(node);
		}

		token_saver.cancel();
		return log.success(node);
	}

	ParseResult Parser::takeStatement(std::span<const Token> &tokens) {
		auto log = logger("takeStatement");

		if (ParseResult node = takeVariableDeclaration(tokens)) {
			return log.success(node);
		}

		if (ParseResult node = takeVariableDefinition(tokens)) {
			return log.success(node);
		}

		if (ParseResult node = takeBlock(tokens)) {
			return log.success(node);
		}

		if (ParseResult node = takeConditional(tokens)) {
			return log.success(node);
		}

		if (ParseResult node = takeReturn(tokens)) {
			return log.success(node);
		}

		if (ParseResult expr = takeExpression(tokens)) {
			if (!take(tokens, TokenType::Semicolon)) {
				return log.fail("Expression statement is missing a semicolon", tokens);
			}

			ASTNodePtr statement = ASTNode::make(NodeType::ExpressionStatement, (*expr)->token);
			(*expr)->reparent(statement);

			return log.success(statement);
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
			node = std::make_shared<TypeNode>(*int_type);
		} else if (const Token *void_type = take(tokens, TokenType::Void)) {
			node = std::make_shared<TypeNode>(*void_type);
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
			node = std::make_shared<TypeNode>(saver->at((pieces.size() - 1) * 2));

			if (pieces.size() == 1) {
				if (!typeDB.contains(name.value())) {
					return log.fail("Not a known type: " + pieces.at(0)->token.value, tokens);
				}
			}
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
						node->add(NodeType::LReference, *ampersand);
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

					node->add(NodeType::LReference, *ampersand);
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

			// TypePtr type = Type::make(std::move(name.value()));
			// typeDB.insert(type);
			// *type_out = QualifiedType(std::move(pointer_consts), is_const, is_reference, std::move(type));

			*type_out = QualifiedType(std::move(pointer_consts), is_const, is_reference, nullptr);
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

	ParseResult Parser::takeVariableDefinition(std::span<const Token> &tokens) {
		auto log = logger("takeVariableDefinition");
		Saver saver{tokens};

		ParseResult variable = takeTypedVariable(tokens);

		if (!variable) {
			return log.fail("No typed variable", tokens, variable);
		}

		const Token *equals = take(tokens, TokenType::Equals);

		if (!equals) {
			return log.fail("No '='", tokens);
		}

		ParseResult expr = takeExpression(tokens);

		if (!expr) {
			return log.fail("No expression", tokens, expr);
		}

		if (!take(tokens, TokenType::Semicolon)) {
			return log.fail("No ';'", tokens);
		}

		ASTNodePtr node = ASTNode::make(NodeType::VariableDefinition, *equals);
		(*variable)->reparent(node);
		(*expr)->reparent(node);

		return log.success(node, saver);
	}

	ParseResult Parser::takeConditional(std::span<const Token> &tokens) {
		auto log = logger("takeConditional");
		Saver saver{tokens};

		const Token *if_token = take(tokens, TokenType::If);

		if (!if_token) {
			return log.fail("No 'if'", tokens);
		}

		ParseResult condition = takeExpression(tokens);

		if (!condition) {
			return log.fail("No expression", tokens, condition);
		}

		ParseResult if_true = takeBlock(tokens);

		if (!if_true) {
			return log.fail("No true block", tokens, if_true);
		}

		if (take(tokens, TokenType::Else)) {
			ParseResult if_false = takeBlock(tokens);

			if (!if_false) {
				return log.fail("No false block", tokens, if_false);
			}

			ASTNodePtr node = ASTNode::make(NodeType::IfStatement, *if_token);
			(*condition)->reparent(node);
			(*if_true)->reparent(node);
			(*if_false)->reparent(node);

			return log.success(node, saver);
		}

		ASTNodePtr node = ASTNode::make(NodeType::IfStatement, *if_token);
		(*condition)->reparent(node);
		(*if_true)->reparent(node);

		return log.success(node, saver);
	}

	ParseResult Parser::takeReturn(std::span<const Token> &tokens) {
		auto log = logger("takeReturn");

		const Token *return_token = take(tokens, TokenType::Return);

		if (!return_token) {
			return log.fail("No 'return'", tokens);
		}

		Saver saver{tokens};

		ParseResult expr = takeExpression(tokens);

		if (!expr) {
			return log.fail("No expression", tokens, expr);
		}

		if (!take(tokens, TokenType::Semicolon)) {
			return log.fail("No ';'", tokens);
		}

		ASTNodePtr node = ASTNode::make(NodeType::ReturnStatement, *return_token);
		(*expr)->reparent(node);

		return log.success(node, saver);
	}

	ParseResult Parser::takeExpression0(std::span<const Token> &tokens) {
		auto log = logger("takeExpression0");

		if (ParseResult expr = takeParenthetical(tokens)) {
			return log.success(expr);
		}

		if (ParseResult expr = takeIdentifier(tokens)) {
			return log.success(expr);
		}

		if (ParseResult expr = takeNumber(tokens)) {
			return log.success(expr);
		}

		if (ParseResult expr = takeString(tokens)) {
			return log.success(expr);
		}

		return log.fail("E0 failed", tokens);
	}

	ParseResult Parser::takeExpression1(std::span<const Token> &tokens) {
		auto log = logger("takeExpression1");

		Saver saver{tokens};

		// E0 E1'
		if (ParseResult deeper = takeExpression0(tokens)) {
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
				return log.success(takePrime1(tokens, node), saver);
			}
		}

		return log.success(lhs);
	}

	ParseResult Parser::takeExpression2(std::span<const Token> &tokens) {
		auto log = logger("takeExpression2");

		Saver saver{tokens};

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

		// E1 E2'
		if (ParseResult deeper = takeExpression1(tokens)) {
			return log.success(takePrime2(tokens, *deeper), saver);
		}

		return log.fail("E2 failed", tokens);
	}

	ParseResult Parser::takePrime2(std::span<const Token> &tokens, const ASTNodePtr &lhs) {
		using enum TokenType;

		auto log = logger("takePrime2");

		Saver saver{tokens};

		// ("++" | "--") E2'
		for (TokenType token_type : {DoublePlus, DoubleMinus}) {
			if (const Token *token = take(tokens, token_type)) {
				NodeType node_type = token_type == DoublePlus? NodeType::PostfixIncrement : NodeType::PostfixDecrement;
				ASTNodePtr node = ASTNode::make(node_type, *token);
				lhs->reparent(node);
				return log.success(takePrime2(tokens, node), saver);
			}
		}

		// "(" Exprs ")" E2'
		if (const Token *opening = take(tokens, OpeningParen)) {
			if (ParseResult args = takeExpressionList(tokens)) {
				if (take(tokens, ClosingParen)) {
					ASTNodePtr node = std::make_shared<FunctionCall>(*opening);
					lhs->reparent(node);
					(*args)->reparent(node);
					return log.success(takePrime2(tokens, node), saver);
				}
			}

			return log.success(lhs);
		}

		// "[" E "]" E2'
		if (const Token *opening = take(tokens, OpeningSquare)) {
			if (ParseResult subscript = takeExpression(tokens)) {
				if (take(tokens, ClosingSquare)) {
					ASTNodePtr node = ASTNode::make(NodeType::Subscript, *opening);
					lhs->reparent(node);
					(*subscript)->reparent(node);
					return log.success(takePrime2(tokens, node), saver);
				}
			}

			return log.success(lhs);
		}

		// "." (ident | "*" | "&") E2'
		if (const Token *dot = take(tokens, Dot)) {
			if (ParseResult ident = takeIdentifier(tokens)) {
				ASTNodePtr node = ASTNode::make(NodeType::AccessMember, *dot);
				lhs->reparent(node);
				(*ident)->reparent(node);
				return log.success(takePrime2(tokens, node), saver);
			}

			if (const Token *star = take(tokens, Star)) {
				ASTNodePtr node = ASTNode::make(NodeType::Deref, *star);
				lhs->reparent(node);
				return log.success(takePrime2(tokens, node), saver);
			}

			if (const Token *ampersand = take(tokens, Ampersand)) {
				ASTNodePtr node = std::make_shared<GetAddress>(*ampersand);
				lhs->reparent(node);
				return log.success(takePrime2(tokens, node), saver);
			}
		}

		return log.success(lhs);
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

		// cast "<" Type ">" "(" E ")"
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

		// "sizeof" "(" E ")"
		if (const Token *sizeof_token = take(tokens, Sizeof)) {
			if (ParseResult subexpr = takeParenthetical(tokens)) {
				ASTNodePtr node = ASTNode::make(NodeType::Sizeof, *sizeof_token);
				(*subexpr)->reparent(node);
				return log.success(node, saver);
			} else {
				return log.fail("Invalid sizeof", tokens, subexpr);
			}
		}

		// "new" Type ("(" Exprs ")" | "[" E "]")?
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
					if (ParseResult count = takeExpression(tokens)) {
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

		// E2
		if (ParseResult deeper = takeExpression2(tokens)) {
			return log.success(deeper, saver);
		}

		return log.fail("E3 failed", tokens);
	}

	ParseResult Parser::takeExpression4(std::span<const Token> &tokens) {
		auto log = logger("takeExpression4");

		// E3 E4'
		if (ParseResult deeper = takeExpression3(tokens)) {
			return log.success(takePrime4(tokens, *deeper));
		} else {
			return log.fail("E4 failed", tokens, deeper);
		}
	}

	ParseResult Parser::takePrime4(std::span<const Token> &tokens, const ASTNodePtr &lhs) {
		using enum TokenType;

		auto log = logger("takePrime4");

		Saver saver{tokens};

		// ("*" | "/" | "%") E3 E4'
		for (TokenType token_type : {Star, Slash, Percent}) {
			if (const Token *token = take(tokens, token_type)) {
				if (ParseResult rhs = takeExpression3(tokens)) {
					ASTNodePtr node = ASTNode::make(NodeType::Binary, *token);
					lhs->reparent(node);
					(*rhs)->reparent(node);
					return log.success(takePrime4(tokens, node), saver);
				}

				return log.success(lhs);
			}
		}

		return log.success(lhs);
	}

	ParseResult Parser::takeExpression5(std::span<const Token> &tokens) {
		auto log = logger("takeExpression5");

		// E4 E5'
		if (ParseResult deeper = takeExpression4(tokens)) {
			return log.success(takePrime5(tokens, *deeper));
		} else {
			return log.fail("E5 failed", tokens, deeper);
		}
	}

	ParseResult Parser::takePrime5(std::span<const Token> &tokens, const ASTNodePtr &lhs) {
		using enum TokenType;

		auto log = logger("takePrime5");

		Saver saver{tokens};

		// ("+" | "-") E4 E5'
		for (TokenType token_type : {Plus, Minus}) {
			if (const Token *token = take(tokens, token_type)) {
				if (ParseResult rhs = takeExpression4(tokens)) {
					ASTNodePtr node = ASTNode::make(NodeType::Binary, *token);
					lhs->reparent(node);
					(*rhs)->reparent(node);
					return log.success(takePrime5(tokens, node), saver);
				}

				return log.success(lhs);
			}
		}

		return log.success(lhs);
	}

	ParseResult Parser::takeExpression6(std::span<const Token> &tokens) {
		auto log = logger("takeExpression6");

		// E5 E6'
		if (ParseResult deeper = takeExpression5(tokens)) {
			return log.success(takePrime6(tokens, *deeper));
		} else {
			return log.fail("E6 failed", tokens, deeper);
		}
	}

	ParseResult Parser::takePrime6(std::span<const Token> &tokens, const ASTNodePtr &lhs) {
		using enum TokenType;

		auto log = logger("takePrime6");

		Saver saver{tokens};

		// ("<<" | ">>") E5 E6'
		for (TokenType token_type : {LeftShift, RightShift}) {
			if (const Token *token = take(tokens, token_type)) {
				if (ParseResult rhs = takeExpression5(tokens)) {
					ASTNodePtr node = ASTNode::make(NodeType::Binary, *token);
					lhs->reparent(node);
					(*rhs)->reparent(node);
					return log.success(takePrime6(tokens, node), saver);
				}

				return log.success(lhs);
			}
		}

		return log.success(lhs);
	}

	ParseResult Parser::takeExpression7(std::span<const Token> &tokens) {
		auto log = logger("takeExpression7");

		// E6 E7'
		if (ParseResult deeper = takeExpression6(tokens)) {
			return log.success(takePrime7(tokens, *deeper));
		} else {
			return log.fail("E7 failed", tokens, deeper);
		}
	}

	ParseResult Parser::takePrime7(std::span<const Token> &tokens, const ASTNodePtr &lhs) {
		using enum TokenType;

		auto log = logger("takePrime7");

		Saver saver{tokens};

		// "<=>" E6 E7'
		if (const Token *token = take(tokens, Spaceship)) {
			if (ParseResult rhs = takeExpression6(tokens)) {
				ASTNodePtr node = ASTNode::make(NodeType::Binary, *token);
				lhs->reparent(node);
				(*rhs)->reparent(node);
				return log.success(takePrime7(tokens, node), saver);
			}

			return log.success(lhs);
		}

		return log.success(lhs);
	}

	ParseResult Parser::takeExpression8(std::span<const Token> &tokens) {
		auto log = logger("takeExpression8");

		// E7 E8'
		if (ParseResult deeper = takeExpression7(tokens)) {
			return log.success(takePrime8(tokens, *deeper));
		} else {
			return log.fail("E8 failed", tokens, deeper);
		}
	}

	ParseResult Parser::takePrime8(std::span<const Token> &tokens, const ASTNodePtr &lhs) {
		using enum TokenType;

		auto log = logger("takePrime8");

		Saver saver{tokens};

		// ("<" | "<=" | ">" | ">=") E7 E8'
		for (TokenType token_type : {OpeningAngle, Leq, ClosingAngle, Geq}) {
			if (const Token *token = take(tokens, token_type)) {
				if (ParseResult rhs = takeExpression7(tokens)) {
					ASTNodePtr node = ASTNode::make(NodeType::Binary, *token);
					lhs->reparent(node);
					(*rhs)->reparent(node);
					return log.success(takePrime8(tokens, node), saver);
				}

				return log.success(lhs);
			}
		}

		return log.success(lhs);
	}

	ParseResult Parser::takeExpression9(std::span<const Token> &tokens) {
		auto log = logger("takeExpression9");

		// E8 E9'
		if (ParseResult deeper = takeExpression8(tokens)) {
			return log.success(takePrime9(tokens, *deeper));
		} else {
			return log.fail("E9 failed", tokens, deeper);
		}
	}

	ParseResult Parser::takePrime9(std::span<const Token> &tokens, const ASTNodePtr &lhs) {
		using enum TokenType;

		auto log = logger("takePrime9");

		Saver saver{tokens};

		// ("==" | "!=") E8 E9'
		for (TokenType token_type : {DoubleEquals, NotEqual}) {
			if (const Token *token = take(tokens, token_type)) {
				if (ParseResult rhs = takeExpression8(tokens)) {
					ASTNodePtr node = ASTNode::make(NodeType::Binary, *token);
					lhs->reparent(node);
					(*rhs)->reparent(node);
					return log.success(takePrime9(tokens, node), saver);
				}

				return log.success(lhs);
			}
		}

		return log.success(lhs);
	}

	ParseResult Parser::takeExpression10(std::span<const Token> &tokens) {
		auto log = logger("takeExpression10");

		// E9 E10'
		if (ParseResult deeper = takeExpression9(tokens)) {
			return log.success(takePrime10(tokens, *deeper));
		} else {
			return log.fail("E10 failed", tokens, deeper);
		}
	}

	ParseResult Parser::takePrime10(std::span<const Token> &tokens, const ASTNodePtr &lhs) {
		using enum TokenType;

		auto log = logger("takePrime10");

		Saver saver{tokens};

		// "&" E9 E10'
		if (const Token *token = take(tokens, Ampersand)) {
			if (ParseResult rhs = takeExpression9(tokens)) {
				ASTNodePtr node = ASTNode::make(NodeType::Binary, *token);
				lhs->reparent(node);
				(*rhs)->reparent(node);
				return log.success(takePrime10(tokens, node), saver);
			}

			return log.success(lhs);
		}

		return log.success(lhs);
	}

	ParseResult Parser::takeExpression11(std::span<const Token> &tokens) {
		auto log = logger("takeExpression11");

		// E10 E11'
		if (ParseResult deeper = takeExpression10(tokens)) {
			return log.success(takePrime11(tokens, *deeper));
		} else {
			return log.fail("E11 failed", tokens, deeper);
		}
	}

	ParseResult Parser::takePrime11(std::span<const Token> &tokens, const ASTNodePtr &lhs) {
		using enum TokenType;

		auto log = logger("takePrime11");

		Saver saver{tokens};

		// "^" E10 E11'
		if (const Token *token = take(tokens, Xor)) {
			if (ParseResult rhs = takeExpression10(tokens)) {
				ASTNodePtr node = ASTNode::make(NodeType::Binary, *token);
				lhs->reparent(node);
				(*rhs)->reparent(node);
				return log.success(takePrime11(tokens, node), saver);
			}

			return log.success(lhs);
		}

		return log.success(lhs);
	}

	ParseResult Parser::takeExpression12(std::span<const Token> &tokens) {
		auto log = logger("takeExpression12");

		// E11 E12'
		if (ParseResult deeper = takeExpression11(tokens)) {
			return log.success(takePrime12(tokens, *deeper));
		} else {
			return log.fail("E12 failed", tokens, deeper);
		}
	}

	ParseResult Parser::takePrime12(std::span<const Token> &tokens, const ASTNodePtr &lhs) {
		using enum TokenType;

		auto log = logger("takePrime12");

		Saver saver{tokens};

		// "|" E11 E12'
		if (const Token *token = take(tokens, Pipe)) {
			if (ParseResult rhs = takeExpression11(tokens)) {
				ASTNodePtr node = ASTNode::make(NodeType::Binary, *token);
				lhs->reparent(node);
				(*rhs)->reparent(node);
				return log.success(takePrime12(tokens, node), saver);
			}

			return log.success(lhs);
		}

		return log.success(lhs);
	}

	ParseResult Parser::takeExpression13(std::span<const Token> &tokens) {
		auto log = logger("takeExpression13");

		// E12 E13'
		if (ParseResult deeper = takeExpression12(tokens)) {
			return log.success(takePrime13(tokens, *deeper));
		} else {
			return log.fail("E13 failed", tokens, deeper);
		}
	}

	ParseResult Parser::takePrime13(std::span<const Token> &tokens, const ASTNodePtr &lhs) {
		using enum TokenType;

		auto log = logger("takePrime13");

		Saver saver{tokens};

		// "&&" E12 E13'
		if (const Token *token = take(tokens, DoubleAmpersand)) {
			if (ParseResult rhs = takeExpression12(tokens)) {
				ASTNodePtr node = ASTNode::make(NodeType::Binary, *token);
				lhs->reparent(node);
				(*rhs)->reparent(node);
				return log.success(takePrime13(tokens, node), saver);
			}

			return log.success(lhs);
		}

		return log.success(lhs);
	}

	ParseResult Parser::takeExpression14(std::span<const Token> &tokens) {
		auto log = logger("takeExpression14");

		// E13 E14'
		if (ParseResult deeper = takeExpression13(tokens)) {
			return log.success(takePrime14(tokens, *deeper));
		} else {
			return log.fail("E14 failed", tokens, deeper);
		}
	}

	ParseResult Parser::takePrime14(std::span<const Token> &tokens, const ASTNodePtr &lhs) {
		using enum TokenType;

		auto log = logger("takePrime14");

		Saver saver{tokens};

		// "||" E13 E14'
		if (const Token *token = take(tokens, DoublePipe)) {
			if (ParseResult rhs = takeExpression13(tokens)) {
				ASTNodePtr node = ASTNode::make(NodeType::Binary, *token);
				lhs->reparent(node);
				(*rhs)->reparent(node);
				return log.success(takePrime14(tokens, node), saver);
			}

			return log.success(lhs);
		}

		return log.success(lhs);
	}

	ParseResult Parser::takeExpression15(std::span<const Token> &tokens) {
		using enum TokenType;

		auto log = logger("takeExpression15");

		Saver saver{tokens};

		if (const Token *if_token = take(tokens, If)) {
			if (ParseResult condition = takeExpression(tokens)) {
				if (ParseResult true_block = takeBlock(tokens)) {
					if (take(tokens, Else)) {
						if (ParseResult false_block = takeBlock(tokens)) {
							ASTNodePtr node = ASTNode::make(NodeType::ConditionalExpression, *if_token);
							(*condition)->reparent(node);
							(*true_block)->reparent(node);
							(*false_block)->reparent(node);
							return log.success(node, saver);
						}
					}
				}
			}

			return log.fail("Invalid conditional expression", tokens);
		}

		if (ParseResult deeper = takeExpression14(tokens)) {
			if (const Token *equals = take(tokens, Equals)) {
				if (ParseResult rhs = takeExpression15(tokens)) {
					ASTNodePtr node = ASTNode::make(NodeType::Assign, *equals);
					(*deeper)->reparent(node);
					(*rhs)->reparent(node);
					return log.success(node, saver);
				} else {
					// We can fail here instead of defaulting to E15 := E14 because "=" isn't valid anywhere else.
					return log.fail("Invalid assignment", tokens, rhs);
				}
			}

			Saver subsaver{tokens};

			static constexpr std::array compounds{
				PlusAssign, MinusAssign, StarAssign, SlashAssign, PercentAssign, LeftShiftAssign, RightShiftAssign,
				AmpersandAssign, XorAssign, PipeAssign, DoubleAmpersandAssign, DoublePipeAssign,
			};

			for (TokenType compound : compounds) {
				if (const Token *compound_token = take(tokens, compound)) {
					if (ParseResult rhs = takeExpression15(tokens)) {
						ASTNodePtr node = ASTNode::make(NodeType::CompoundAssign, *compound_token);
						(*deeper)->reparent(node);
						(*rhs)->reparent(node);
						subsaver.cancel();
						return log.success(node, saver);
					} else {
						subsaver.restore();
					}
				}
			}

			return log.success(deeper, saver);
		}

		return log.fail("E15 failed", tokens);
	}

	ParseResult Parser::takeExpression16(std::span<const Token> &tokens) {
		auto log = logger("takeExpression16");

		// E15 E16'
		if (ParseResult deeper = takeExpression15(tokens)) {
			return log.success(takePrime16(tokens, *deeper));
		} else {
			return log.fail("E16 failed", tokens, deeper);
		}
	}

	ParseResult Parser::takePrime16(std::span<const Token> &tokens, const ASTNodePtr &lhs) {
		auto log = logger("takePrime16");

		Saver saver{tokens};

		if (commaAllowed) {
			if (const Token *comma = take(tokens, TokenType::Comma)) {
				if (ParseResult rhs = takeExpression15(tokens)) {
					ASTNodePtr node = ASTNode::make(NodeType::Comma, *comma);
					lhs->reparent(node);
					(*rhs)->reparent(node);
					return log.success(takePrime16(tokens, node), saver);
				}
			}
		}

		return log.success(lhs);
	}

	ParseResult Parser::takeExpressionList(std::span<const Token> &tokens) {
		auto log = logger("takeExpressionList");

		if (tokens.empty()) {
			return log.fail("No tokens", tokens);
		}

		Saver token_saver{tokens};

		Saver comma_saver{commaAllowed};
		commaAllowed = false;

		ASTNodePtr node = ASTNode::make(NodeType::Expressions, tokens.front());

		if (ParseResult first = takeExpression(tokens)) {
			(*first)->reparent(node);

			for (;;) {
				Saver subsaver{tokens};

				if (take(tokens, TokenType::Comma)) {
					if (ParseResult next = takeExpression(tokens)) {
						(*next)->reparent(node);
						subsaver.cancel();
					} else {
						break;
					}
				} else {
					break;
				}
			}
		}

		return log.success(node, token_saver);
	}

	ParseResult Parser::takeExpression(std::span<const Token> &tokens) {
		return takeExpression16(tokens);
	}
}
