#include "Reading.h"
#include <map>

struct CardAssociations {
	BString astrological;
	BString hebrewLetter;
	BString meaning;
};

static std::map<BString, CardAssociations> cardData = {
	{"1 The Magician",
		{"Mercury", "Beth",
			"Represents the conscious mind, the intellect, and the power of manifestation."}},
	{"2 The High Priestess",
		{"Moon", "Gimel", "Represents the subconscious mind, intuition, and hidden knowledge."}},
	{"3 The Empress",
		{"Venus", "Daleth", "Represents motherhood, fertility, and the abundance of nature."}},
	{"4 The Emperor",
		{"Aries", "Heh", "Represents fatherhood, authority, and the structure of society."}},
	{"5 The Hierophant",
		{"Taurus", "Vav", "Represents tradition, religion, and the pursuit of knowledge."}},
	{"6 The Lovers",
		{"Gemini", "Zain", "Represents relationships, choices, and the union of opposites."}},
	{"7 The Chariot",
		{"Cancer", "Cheth", "Represents victory, willpower, and the mastery of opposing forces."}},
	{"8 Strength",
		{"Leo", "Teth",
			"Represents courage, passion, and the integration of the conscious and subconscious "
			"minds."}},
	{"9 The Hermit",
		{"Virgo", "Yod", "Represents introspection, solitude, and the search for inner wisdom."}},
	{"10 Wheel Of Fortune",
		{"Jupiter", "Kaph", "Represents cycles, destiny, and the ever-changing nature of life."}},
	{"11 Justice",
		{"Libra", "Lamed", "Represents fairness, balance, and the consequences of one's actions."}},
	{"12 The Hanged Man",
		{"Neptune", "Mem", "Represents sacrifice, surrender, and a change in perspective."}},
	{"13 Death", {"Scorpio", "Nun", "Represents transformation, endings, and new beginnings."}},
	{"14 Temperance",
		{"Sagittarius", "Samekh",
			"Represents balance, harmony, and the integration of opposites."}},
	{"15 The Devil",
		{"Capricorn", "Ayin",
			"Represents materialism, addiction, and the darker aspects of human nature."}},
	{"16 The Tower",
		{"Mars", "Peh",
			"Represents sudden change, upheaval, and the destruction of old structures."}},
	{"17 The Star",
		{"Aquarius", "Tzaddi", "Represents hope, inspiration, and the connection to the divine."}},
	{"18 The Moon",
		{"Pisces", "Qoph",
			"Represents intuition, dreams, and the exploration of the subconscious."}},
	{"19 The Sun", {"Sun", "Resh", "Represents joy, vitality, and the celebration of life."}},
	{"20 Judgement",
		{"Pluto", "Shin", "Represents resurrection, forgiveness, and the final judgment."}},
	{"21 The World",
		{"Saturn", "Tav",
			"Represents completion, fulfillment, and the integration of all aspects of the self."}},
	{"Ace Of Cups",
		{"Water", "Heh",
			"Represents the beginning of a new emotional cycle, the potential for love and "
			"happiness."}},
	{"Two Of Cups",
		{"Venus in Cancer", "Cheth",
			"Represents a deep connection between two people, a partnership based on love and "
			"mutual respect."}},
	{"Three Of Cups",
		{"Mercury in Cancer", "Cheth",
			"Represents a celebration of friendship, a time of joy and shared happiness."}},
	{"Four Of Cups",
		{"Moon in Cancer", "Cheth",
			"Represents a time of emotional withdrawal, a need to re-evaluate one's feelings."}},
	{"Five Of Cups",
		{"Mars in Scorpio", "Nun",
			"Represents a sense of loss and disappointment, a need to grieve and move on."}},
	{"Six Of Cups",
		{"Sun in Scorpio", "Nun",
			"Represents a return to the past, a time of nostalgia and happy memories."}},
	{"Seven Of Cups",
		{"Venus in Scorpio", "Nun",
			"Represents a time of wishful thinking, a need to make a choice and take action."}},
	{"Eight Of Cups",
		{"Saturn in Pisces", "Qoph",
			"Represents a time of emotional transition, a need to move on from the past."}},
	{"Nine Of Cups",
		{"Jupiter in Pisces", "Qoph",
			"Represents a time of emotional fulfillment, a wish come true."}},
	{"Ten Of Cups",
		{"Mars in Pisces", "Qoph",
			"Represents a time of lasting happiness, a deep sense of emotional security."}},
	{"Page Of Cups",
		{"Earth of Water", "Heh", "Represents a message of love, a new emotional beginning."}},
	{"Knight Of Cups",
		{"Air of Water", "Heh", "Represents a romantic proposal, a journey of the heart."}},
	{"Queen Of Cups",
		{"Water of Water", "Heh",
			"Represents a compassionate and intuitive woman, a loving mother and wife."}},
	{"King Of Cups",
		{"Fire of Water", "Heh",
			"Represents a mature and emotionally balanced man, a wise and compassionate leader."}},
	{"Ace Of Pentacles",
		{"Earth", "Heh",
			"Represents the beginning of a new financial cycle, the potential for prosperity and "
			"abundance."}},
	{"Two Of Pentacles",
		{"Jupiter in Capricorn", "Ayin",
			"Represents a time of financial juggling, a need to balance competing priorities."}},
	{"Three Of Pentacles",
		{"Mars in Capricorn", "Ayin",
			"Represents a time of collaboration and teamwork, a successful project."}},
	{"Four Of Pentacles",
		{"Sun in Capricorn", "Ayin",
			"Represents a time of financial security, a need to hold on to what one has."}},
	{"Five Of Pentacles",
		{"Mercury in Taurus", "Vav",
			"Represents a time of financial hardship, a need to seek help from others."}},
	{"Six Of Pentacles",
		{"Moon in Taurus", "Vav",
			"Represents a time of generosity and charity, a need to share one's wealth."}},
	{"Seven Of Pentacles",
		{"Saturn in Taurus", "Vav",
			"Represents a time of patience and perseverance, a need to wait for the harvest."}},
	{"Eight Of Pentacles",
		{"Sun in Virgo", "Yod",
			"Represents a time of hard work and dedication, a commitment to one's craft."}},
	{"Nine Of Pentacles",
		{"Venus in Virgo", "Yod",
			"Represents a time of financial independence, a life of luxury and refinement."}},
	{"Ten Of Pentacles",
		{"Mercury in Virgo", "Yod",
			"Represents a time of family wealth and security, a legacy for future generations."}},
	{"Page Of Pentacles",
		{"Earth of Earth", "Heh",
			"Represents a message of financial opportunity, a new job or investment."}},
	{"Knight Of Pentacles",
		{"Air of Earth", "Heh",
			"Represents a practical and reliable young man, a hard worker with a strong sense of "
			"duty."}},
	{"Queen Of Pentacles",
		{"Water of Earth", "Heh",
			"Represents a nurturing and practical woman, a successful businesswoman and a loving "
			"mother."}},
	{"King Of Pentacles",
		{"Fire of Earth", "Heh",
			"Represents a wealthy and successful man, a generous and responsible leader."}},
	{"Ace Of Swords",
		{"Air", "Vav",
			"Represents the beginning of a new intellectual cycle, the potential for clarity and "
			"truth."}},
	{"Two Of Swords",
		{"Moon in Libra", "Lamed",
			"Represents a time of indecision, a need to make a difficult choice."}},
	{"Three Of Swords",
		{"Saturn in Libra", "Lamed",
			"Represents a time of heartbreak and sorrow, a painful but necessary separation."}},
	{"Four Of Swords",
		{"Jupiter in Libra", "Lamed",
			"Represents a time of rest and recuperation, a need to withdraw from the world."}},
	{"Five Of Swords",
		{"Venus in Aquarius", "Tzaddi",
			"Represents a time of conflict and defeat, a need to surrender and move on."}},
	{"Six Of Swords",
		{"Mercury in Aquarius", "Tzaddi",
			"Represents a time of transition and change, a journey to a better place."}},
	{"Seven Of Swords",
		{"Moon in Aquarius", "Tzaddi",
			"Represents a time of deception and betrayal, a need to be cautious and alert."}},
	{"Eight Of Swords",
		{"Jupiter in Gemini", "Zain",
			"Represents a time of restriction and confinement, a need to break free from "
			"self-imposed limitations."}},
	{"Nine Of Swords",
		{"Mars in Gemini", "Zain",
			"Represents a time of anxiety and despair, a need to face one's fears."}},
	{"Ten Of Swords",
		{"Sun in Gemini", "Zain",
			"Represents a time of complete and utter ruin, the end of a difficult cycle."}},
	{"Page Of Swords",
		{"Earth of Air", "Vav",
			"Represents a message of conflict and challenge, a need to be assertive and stand up "
			"for oneself."}},
	{"Knight Of Swords",
		{"Air of Air", "Vav",
			"Represents a young man who is intelligent and ambitious, but also reckless and "
			"impulsive."}},
	{"Queen Of Swords",
		{"Water of Air", "Vav",
			"Represents a woman who is intelligent and independent, but also cold and aloof."}},
	{"King Of Swords",
		{"Fire of Air", "Vav",
			"Represents a man who is intelligent and authoritative, but also ruthless and "
			"judgmental."}},
	{"Ace Of Wands",
		{"Fire", "Yod",
			"Represents the beginning of a new creative cycle, the potential for growth and "
			"expansion."}},
	{"Two Of Wands",
		{"Mars in Aries", "Heh",
			"Represents a time of planning and preparation, a need to make a choice between two "
			"paths."}},
	{"Three Of Wands",
		{"Sun in Aries", "Heh",
			"Represents a time of exploration and discovery, a need to expand one's horizons."}},
	{"Four Of Wands",
		{"Venus in Aries", "Heh",
			"Represents a time of celebration and joy, a happy and harmonious home life."}},
	{"Five Of Wands",
		{"Saturn in Leo", "Teth",
			"Represents a time of competition and conflict, a need to prove oneself."}},
	{"Six Of Wands",
		{"Jupiter in Leo", "Teth",
			"Represents a time of victory and success, a public recognition of one's "
			"achievements."}},
	{"Seven Of Wands",
		{"Mars in Leo", "Teth",
			"Represents a time of courage and defiance, a need to stand up for one's beliefs."}},
	{"Eight Of Wands",
		{"Mercury in Sagittarius", "Samekh",
			"Represents a time of rapid progress and communication, a sudden burst of energy."}},
	{"Nine Of Wands",
		{"Moon in Sagittarius", "Samekh",
			"Represents a time of strength and resilience, a need to persevere in the face of "
			"adversity."}},
	{"Ten Of Wands",
		{"Saturn in Sagittarius", "Samekh",
			"Represents a time of burden and responsibility, a need to delegate and ask for "
			"help."}},
	{"Page Of Wands",
		{"Earth of Fire", "Yod",
			"Represents a message of creative inspiration, a new project or idea."}},
	{"Knight Of Wands",
		{"Air of Fire", "Yod",
			"Represents a young man who is energetic and adventurous, but also reckless and "
			"impulsive."}},
	{"Queen Of Wands",
		{"Water of Fire", "Yod",
			"Represents a woman who is confident and charismatic, but also hot-tempered and "
			"demanding."}},
	{"King Of Wands",
		{"Fire of Fire", "Yod",
			"Represents a man who is a natural leader, a visionary with a passion for life."}}};


Reading::Reading(const std::vector<BString>& cardNames)
	:
	fCardNames(cardNames)
{
}


BString
Reading::GetInterpretation()
{
	return GenerateInterpretation();
}


BString
Reading::GenerateInterpretation()
{
	BString interpretation = "";
	for (const BString& cardName : fCardNames) {
		if (cardData.count(cardName) > 0) {
			interpretation << "Card: " << cardName << "\n";
			interpretation << "Astrological Sign: " << cardData[cardName].astrological << "\n";
			interpretation << "Hebrew Letter: " << cardData[cardName].hebrewLetter << "\n";
			interpretation << "Meaning: " << cardData[cardName].meaning << "\n\n";
		}
	}
	return interpretation;
}
