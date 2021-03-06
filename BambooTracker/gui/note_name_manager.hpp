/*
 * Copyright (C) 2021 Rerrah
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef NOTE_NAME_MANAGER_HPP
#define NOTE_NAME_MANAGER_HPP

#include <memory>
#include <QString>
#include <QStringList>
#include "song.hpp"

enum class NoteNotationSystem : int;

class NoteNameManager final
{
public:
	static NoteNameManager& getManager();
	~NoteNameManager();

	void setNotationSystem(NoteNotationSystem system);

	QString getNoteName(int n, KeySignature::Type key = KeySignature::E) const;
	QString getNoteString(int noteNum, KeySignature::Type key = KeySignature::E) const;

	QString getKeyName(KeySignature::Type key) const;

private:
	static std::unique_ptr<NoteNameManager> instance_;

	NoteNameManager();
	const std::unordered_map<KeySignature::Type, QStringList>* list_;

};

#endif // NOTE_NAME_MANAGER_HPP
