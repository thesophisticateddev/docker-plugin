/*
    SPDX-FileCopyrightText: 2026 Salman Ahmed <salman.sc829@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef DOCKER_CLIENT_H
#define DOCKER_CLIENT_H

#include <QObject>
#include <QString>
#include <QList>

class QProcess;

struct ContainerInfo {
    QString id;
    QString name;
    QString image;
    QString status;
    QString ports;
};

enum class DockerAvailability {
    Unknown,
    Available,
    NotFound,
    DaemonDown,
    PermissionDenied,
    Error
};

class DockerClient : public QObject
{
    Q_OBJECT

public:
    explicit DockerClient(QObject *parent = nullptr);
    ~DockerClient() override;

    void checkAvailability();
    void listContainers(bool showAll = false);

Q_SIGNALS:
    void availabilityChanged(DockerAvailability availability, const QString &errorMessage);
    void containersUpdated(const QList<ContainerInfo> &containers);

private:
    DockerAvailability classifyError(const QString &stdErr) const;
    QList<ContainerInfo> parseContainerOutput(const QByteArray &data) const;
};

#endif // DOCKER_CLIENT_H
