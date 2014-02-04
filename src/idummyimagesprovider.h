#ifndef IDUMMYIMAGESPROVIDER_H
#define IDUMMYIMAGESPROVIDER_H

#include <QPixmap>

namespace qNotesManager {
	class IDummyImagesProvider {
	public:
		virtual QPixmap GetErrorImage() const = 0;
		virtual QPixmap GetLoadingImage() const = 0;
	};
}

#endif // IDUMMYIMAGESPROVIDER_H
