#ifndef CSV_FOREIGN_REFERENCECREATOR_H
#define CSV_FOREIGN_REFERENCECREATOR_H

#include "../world/genericcreator.hpp"

namespace CSMDoc
{
    class Document;
}

namespace CSMWorld
{
    class IdCompletionManager;
    class UniversalId;
    class CreateCommand;
    class Creator;
}

namespace CSVWidget
{
    class DropLineEdit;
}

namespace CSVForeign
{

    class ReferenceCreator : public CSVWorld::GenericCreator
    {
            Q_OBJECT

            CSVWidget::DropLineEdit *mCell;
            std::string mId;

        private:

            virtual std::string getId() const;

            virtual void configureCreateCommand (CSMWorld::CreateCommand& command) const;

            virtual void pushCommand (std::auto_ptr<CSMWorld::CreateCommand> command,
                const std::string& id);

            int getRefNumCount() const;

        public:

            ReferenceCreator (CSMWorld::Data& data, QUndoStack& undoStack,
                const CSMWorld::UniversalId& id, CSMWorld::IdCompletionManager& completionManager);

            virtual void cloneMode(const std::string& originId,
                                   const CSMWorld::UniversalId::Type type);

            virtual void reset();

            virtual std::string getErrors() const;
            ///< Return formatted error descriptions for the current state of the creator. if an empty
            /// string is returned, there is no error.

            /// Focus main input widget
            virtual void focus();

        private slots:

            void cellChanged();
    };

    class ReferenceCreatorFactory : public CSVWorld::CreatorFactoryBase
    {
        public:

            virtual CSVWorld::Creator *makeCreator (CSMDoc::Document& document, const CSMWorld::UniversalId& id) const;
            ///< The ownership of the returned Creator is transferred to the caller.
    };
}

#endif