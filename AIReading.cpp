#include "AIReading.h"
#include "CardModel.h"
#include "Config.h"
#include "HTTPClient.h"
#include "JSONParser.h"

#include <iostream>
#include <stdlib.h>
#include <string.h>


BString
AIReading::GetReading(const std::vector<CardInfo>& cards, SpreadType spreadType)
{
	if (!Config::IsAPIKeySet()) {
		BString message
			= "DeepSeek API key not set. Please set the DEEPSEEK_API_KEY environment variable.\n\n";
		message += "Card spread: ";

		for (size_t i = 0; i < cards.size(); i++) {
			if (i > 0)
				message += ", ";
			message += cards[i].displayName;
		}

		return message;
	}


	BString prompt;
	if (spreadType == THREE_CARD) {
		prompt = "Provide a tarot card reading for the following three cards drawn in a simple "
				 "spread: ";
		for (size_t i = 0; i < cards.size(); ++i)
			prompt << "\n- " << cards[i].displayName;
	} else if (spreadType == TREE_OF_LIFE) {
		prompt = "Provide a tarot card reading for the following ten cards drawn in a Tree of Life "
				 "spread: ";
		const char* positions[] = {"1. Kether (The Crown) - Highest spiritual aspirations",
			"2. Chokmah (Wisdom) - Spiritual potential realized",
			"3. Binah (Understanding) - Spiritual limitations and restrictions",
			"4. Chesed (Mercy) - Constructive influences and prosperity",
			"5. Geburah (Severity) - Destructive influences and misuse of power",
			"6. Tiphareth (Beauty) - Your true self, the heart of the matter",
			"7. Netzach (Victory) - Your emotional state, love, and passion",
			"8. Hod (Splendor) - Your intellectual state, communication, and work",
			"9. Yesod (Foundation) - Your unconscious state, intuition, and dreams",
			"10. Malkuth (Kingdom) - The final outcome and material manifestation"};
		for (size_t i = 0; i < cards.size() && i < 10; ++i)
			prompt << "\n- " << positions[i] << ": " << cards[i].displayName;
	}

	prompt
		<< ". Give a detailed, insightful reading focusing on the combined meaning of these cards. "
		   "Provide a comprehensive interpretation that explores the nuances of the cards' "
		   "interactions. "
		   "Keep the response to 5-7 sentences. Do not use markdown or any special formatting.";

	BString jsonPayload
		= JSONParser::BuildPayload(prompt, Config::kAPIMaxTokens, Config::kAPITemperature);


	BString authHeader = "Bearer ";
	authHeader += Config::GetAPIKey();


	HTTPClient httpClient;
	BString response
		= httpClient.Post("https://api.deepseek.com/v1/chat/completions", jsonPayload, authHeader);


	return JSONParser::ParseAPIResponse(response);
}
