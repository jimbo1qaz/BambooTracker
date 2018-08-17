#ifndef SET_EFFECT_VALUE_TO_STEP_QT_COMMAND_HPP
#define SET_EFFECT_VALUE_TO_STEP_QT_COMMAND_HPP

#include <QUndoCommand>
#include "gui/pattern_editor/pattern_editor_panel.hpp"
#include "gui/pattern_editor/pattern_position.hpp"

class SetEffectValueToStepQtCommand : public QUndoCommand
{
public:
	SetEffectValueToStepQtCommand(PatternEditorPanel* panel, PatternPosition pos, QUndoCommand* parent = nullptr);
	void redo() Q_DECL_OVERRIDE;
	void undo() Q_DECL_OVERRIDE;
	int id() const Q_DECL_OVERRIDE;
	bool mergeWith(const QUndoCommand* other) Q_DECL_OVERRIDE;

	PatternPosition getPos() const;

private:
	PatternEditorPanel* panel_;
	PatternPosition pos_;
	bool isComplete_;
};

#endif // SET_EFFECT_VALUE_TO_STEP_QT_COMMAND_HPP
